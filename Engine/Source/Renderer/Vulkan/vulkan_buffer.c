#include "vulkan_buffer.h"

#include "core/logger.h"
#include "systems/memory_system.h"
#include "memory/dynamic_allocator.h"
#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"

static void* VKAPI_CALL allocation(void* user_data, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope);
static void* VKAPI_CALL reallocation(void* user_data, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope);
static void VKAPI_CALL free(void* user_data, void* memory);
static void destroy_suballocation_tracker(vulkan_buffer* buffer);

b8 vulkan_buffer_create(vulkan_context* context, u64 size, VkBufferUsageFlagBits usage, u32 memory_property, b8 bind_on_create, vulkan_buffer* buffer)
{
    memory_zero(buffer, sizeof(*buffer));
    buffer->size = size;
    buffer->usage = usage;
    buffer->memory_property = memory_property;

    buffer->suballocation.required_memory = 0;
    freelist_create(&buffer->suballocation.required_memory, 0, buffer->size, 0);
    buffer->suballocation.block = memory_system_allocate(buffer->suballocation.required_memory, MEMORY_TAG_RENDERER);
    freelist_create(&buffer->suballocation.required_memory, buffer->suballocation.block, buffer->size, &buffer->suballocation.tracker);

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = 0;
    createInfo.flags = 0;
    createInfo.size = buffer->size;
    createInfo.usage = buffer->usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = 0;
    VK_CHECK(vkCreateBuffer(context->device.handle, &createInfo, context->allocator, &buffer->handle));

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(context->device.handle, buffer->handle, &memory_requirements);
    buffer->memory_index = context->find_memory_type_index(memory_requirements.memoryTypeBits, buffer->memory_property);
    if (buffer->memory_index == -1)
    {
        LOG_ERROR("vulkan_buffer_create: Failed to find memory type index");
        destroy_suballocation_tracker(buffer);
        return FALSE;
    }

    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = 0;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = buffer->memory_index;
    VK_CHECK(vkAllocateMemory(context->device.handle, &allocate_info, context->allocator, &buffer->device_memory));
    VK_CHECK(vkBindBufferMemory(context->device.handle, buffer->handle, buffer->device_memory, 0));

    return TRUE;
}
void vulkan_buffer_destroy(vulkan_context* context, vulkan_buffer* buffer)
{
    if (buffer->suballocation.block)
    {
        destroy_suballocation_tracker(buffer);
    }

    if (buffer->device_memory)
    {
        vkFreeMemory(context->device.handle, buffer->device_memory, context->allocator);
        buffer->device_memory = 0;
    }

    if (buffer->handle)
    {
        vkDestroyBuffer(context->device.handle, buffer->handle, context->allocator);
        buffer->handle = 0;
    }

    buffer->size = 0;
    buffer->usage = 0;
    buffer->locked = FALSE;
}

b8 vulkan_buffer_resize(vulkan_context* context, vulkan_buffer* buffer, u64 new_size, VkCommandPool pool, VkQueue queue)
{
    if (new_size <= buffer->size)
    {
        LOG_ERROR("vulkan_buffer_resize: Invalid input parameters");
        return FALSE;
    }

    freelist_resize(&buffer->suballocation.tracker, new_size);

    vulkan_buffer new_buffer;
    vulkan_buffer_create(context, new_size, buffer->usage, buffer->memory_property, TRUE, &new_buffer);
    vulkan_buffer_copy_to_buffer(context, pool, 0, queue, new_buffer.handle, 0, buffer->handle, 0, buffer->size);

    vkDeviceWaitIdle(context->device.handle);

    buffer->handle = new_buffer.handle;
    buffer->size = new_buffer.size;
    buffer->device_memory = new_buffer.device_memory;

    vulkan_buffer_destroy(context, &new_buffer);
    return TRUE;
}

void vulkan_buffer_bind(vulkan_context* context, vulkan_buffer* buffer, u64 offset)
{
    VK_CHECK(vkBindBufferMemory(context->device.handle, buffer->handle, buffer->device_memory, offset));
}

void vulkan_buffer_upload_to_host_visible_memory(vulkan_context* context, vulkan_buffer* buffer, u64 offset, u64 size, u32 flags, void const* data)
{
    void* mapped;
    VK_CHECK(vkMapMemory(context->device.handle, buffer->device_memory, 0, buffer->size, 0, &mapped));
    memory_copy(mapped, data, size);
    vkUnmapMemory(context->device.handle, buffer->device_memory);
}

void vulkan_buffer_copy_to_buffer(
    vulkan_context* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer source,
    u64 source_offset,
    VkBuffer dest,
    u64 dest_offset,
    u64 size)
{
    vkQueueWaitIdle(queue);
    vulkan_command_buffer temp_command_buffer;
    vulkan_command_buffer_allocate_and_begin_single_use(context, pool, &temp_command_buffer);

    VkBufferCopy buffer_copy = {};
    buffer_copy.srcOffset = source_offset;
    buffer_copy.dstOffset = dest_offset;
    buffer_copy.size = size;

    vkCmdCopyBuffer(temp_command_buffer.handle, source, dest, 1, &buffer_copy);
    vulkan_command_buffer_end_single_use(context, pool, &temp_command_buffer, queue);
}

void vulkan_buffer_upload_to_device_local_memory(
    vulkan_context* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    vulkan_buffer* buffer,
    u64 offset,
    u64 size,
    void const* data)
{
    vulkan_buffer staging;
    VkMemoryPropertyFlagBits memory_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vulkan_buffer_create(context, size, usage, memory_properties, TRUE, &staging);
    vulkan_buffer_upload_to_host_visible_memory(context, &staging, 0, size, 0, data);
    vulkan_buffer_copy_to_buffer(context, pool, fence, queue, staging.handle, 0, buffer->handle, offset, size);
    vulkan_buffer_destroy(context, &staging);
}

void vulkan_buffer_free_data(vulkan_buffer* buffer, u64 offset, u64 size) {
    // TODO: Free this in the buffer.
    // TODO: update free list with this range being free.
}

b8 vulkan_buffer_suballocate(vulkan_buffer* buffer, u64 size, u64* offset)
{
    if (!buffer || (size == 0) || !offset)
    {
        LOG_ERROR("vulkan_buffer_suballocate: Invalid input parameters");
        return FALSE;
    }

    return freelist_allocate(&buffer->suballocation.tracker, size, offset);
}

b8 vulkan_buffer_free(vulkan_buffer* buffer, u64 size, u64 offset)
{
    if (!buffer || (size == 0))
    {
        LOG_ERROR("vulkan_buffer_free: Invalid input parameters");
        return FALSE;
    }

    return freelist_free(&buffer->suballocation.tracker, size, offset);
}

void* VKAPI_CALL allocation(void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
    return memory_allocate(size, MEMORY_TAG_RENDERER);
}

void* VKAPI_CALL reallocation(void* user_data, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope)
{
    return memory_allocate(size, MEMORY_TAG_RENDERER);
}

void VKAPI_CALL free(void* user_data, void* memory)
{
    return memory_free(memory, 0, MEMORY_TAG_RENDERER);
}

void destroy_suballocation_tracker(vulkan_buffer* buffer)
{
    freelist_destroy(&buffer->suballocation.tracker);
    memory_system_free(buffer->suballocation.block, buffer->suballocation.required_memory, MEMORY_TAG_RENDERER);
    buffer->suballocation.required_memory = 0;
    buffer->suballocation.block = 0;
}

#include "vulkan_buffer.h"
#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"
#include "Core/logger.h"
#include "Core/memory.h"

b8 vulkan_buffer_create(
    vulkan_context* context,
    u64 size,
    VkBufferUsageFlagBits usage,
    u32 memory_property_flags,
    b8 bind_on_create,
    vulkan_buffer* buffer)
{
    memory_zero(buffer, sizeof(*buffer));
    buffer->size = size;
    buffer->usage = usage;
    buffer->memory_properties = memory_property_flags;

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
    buffer->memory_index = context->findMemoryType(memory_requirements.memoryTypeBits, buffer->memory_properties);

    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = 0;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = buffer->memory_index;
    VK_CHECK(vkAllocateMemory(context->device.handle, &allocate_info, context->allocator, &buffer->memory));

    VK_CHECK(vkBindBufferMemory(context->device.handle, buffer->handle, buffer->memory, 0));

    return TRUE;
}
void vulkan_buffer_destroy(vulkan_context* context, vulkan_buffer* buffer)
{
    if (buffer->memory) {
        vkFreeMemory(context->device.handle, buffer->memory, context->allocator);
        buffer->memory = 0;
    }

    if (buffer->handle) {
        vkDestroyBuffer(context->device.handle, buffer->handle, context->allocator);
        buffer->handle = 0;
    }

    buffer->size = 0;
    buffer->usage = 0;
    buffer->locked = FALSE;
}

b8 vulkan_buffer_resize(
    vulkan_context* context,
    vulkan_buffer* buffer,
    u64 new_size,
    VkCommandPool pool,
    VkQueue queue)
{
    vulkan_buffer new_buffer;
    vulkan_buffer_create(context, new_size, buffer->usage, buffer->memory_properties, TRUE, &new_buffer);
    vulkan_buffer_copy(context, pool, 0, queue, new_buffer.handle, 0, buffer->handle, 0, buffer->size);

    vkDeviceWaitIdle(context->device.handle);

    buffer->handle = new_buffer.handle;
    buffer->size = new_buffer.size;
    buffer->memory = new_buffer.memory;

    vulkan_buffer_destroy(context, &new_buffer);
    return TRUE;
}

void vulkan_buffer_bind(vulkan_context* context, vulkan_buffer* buffer, u64 offset)
{
    VK_CHECK(vkBindBufferMemory(context->device.handle, buffer->handle, buffer->memory, offset));
}

void* vulkan_buffer_map_memory(
    vulkan_context* context,
    vulkan_buffer* buffer,
    u64 offset,
    u64 size,
    u32 flags)
{
    VK_CHECK(vkMapMemory(context->device.handle, buffer->memory, 0, buffer->size, 0, &buffer->mapped));
    return buffer->mapped;
}

void vulkan_buffer_unmap_memory(vulkan_context* context, vulkan_buffer* buffer)
{
    vkUnmapMemory(context->device.handle, buffer->memory);
}

void vulkan_buffer_load_data(
    vulkan_context* context,
    vulkan_buffer* buffer,
    u64 offset,
    u64 size,
    u32 flags,
    void const* data)
{
    vulkan_buffer_map_memory(context, buffer, offset, size, flags);
    memory_copy(buffer->alignment, data, size);
    vulkan_buffer_unmap_memory(context, buffer);
}

void vulkan_buffer_copy(
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

    vulkan_command_buffer temp_comman_buffer;
    vulkan_command_buffer_allocate_and_begin_single_use(context, pool, &temp_comman_buffer);

    VkBufferCopy buffer_copy = {};
    buffer_copy.srcOffset = source_offset;
    buffer_copy.dstOffset = dest_offset;
    buffer_copy.size = size;

    vkCmdCopyBuffer(temp_comman_buffer.handle, source, dest, 1, &buffer_copy);

    vulkan_command_buffer_end_single_use(context, pool, &temp_comman_buffer, queue);
}

void vulkan_buffer_upload_data(
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
    VkMemoryPropertyFlagBits memory_properties =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vulkan_buffer_create(context, size, usage, memory_properties, TRUE, &staging);

    vulkan_buffer_load_data(context, &staging, 0, size, 0, data);
    vulkan_buffer_copy(context, pool, fence, queue, staging.handle, 0, buffer->handle, offset, size);
    vulkan_buffer_destroy(context, &staging);
}

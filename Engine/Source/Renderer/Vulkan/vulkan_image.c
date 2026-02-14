#include "vulkan_image.h"

#include "vulkan_device.h"
#include "core/logger.h"

void vulkan_image_create(vulkan_context* context, VkImageType imageType, u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, b32 createView, VkImageAspectFlags aspectFlags, vulkan_image* image)
{
    image->width = width;
    image->height = height;

    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = format;
    createInfo.extent.width = width;
    createInfo.extent.height = height;
    createInfo.extent.depth = 1;
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = tiling;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = 0;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VULKAN_CHECK_RESULT(vkCreateImage(context->device.handle, &createInfo, context->allocator, &image->handle));

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(context->device.handle, image->handle, &memoryRequirements);

    i32 memoryType = context->find_memory_type_index(memoryRequirements.memoryTypeBits, memoryProperties);
    if (memoryType == -1) {
        LOG_ERROR("Required memory type not found. Image not valid");
    }

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = NULL;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = memoryType;
    VULKAN_CHECK_RESULT(vkAllocateMemory(context->device.handle, &allocateInfo, context->allocator, &image->memory));
    VULKAN_CHECK_RESULT(vkBindImageMemory(context->device.handle, image->handle, image->memory, 0));

    if (createView) {
        image->view = VK_NULL_HANDLE;
        vulkan_image_view_create(context, format, image, aspectFlags);
    }
}

void vulkan_image_view_create(
    vulkan_context* context,
    VkFormat format,
    vulkan_image* image,
    VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.image = image->handle;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    VULKAN_CHECK_RESULT(vkCreateImageView(context->device.handle, &createInfo, context->allocator, &image->view));
}

void vulkan_image_destroy(vulkan_context* context, vulkan_image* image)
{
    if (image->view) {
        vkDestroyImageView(context->device.handle, image->view, context->allocator);
        image->view = VK_NULL_HANDLE;
    }
    if (image->memory) {
        vkFreeMemory(context->device.handle, image->memory, context->allocator);
        image->memory = VK_NULL_HANDLE;
    }
    if (image->handle) {
        vkDestroyImage(context->device.handle, image->handle, context->allocator);
        image->handle = VK_NULL_HANDLE;
    }
}

void vulkan_image_transition_layout(
    vulkan_context* context,
    vulkan_command_buffer* command_buffer,
    vulkan_image* image,
    VkFormat format,
    VkImageLayout old_layout,
    VkImageLayout new_layout)
{
    VkAccessFlags src_access_mask;
    VkAccessFlags dst_access_mask;
    VkPipelineStageFlags src_stage_mask;
    VkPipelineStageFlags dst_stage_mask;
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        src_access_mask = 0;
        dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
        src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (
        old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
        src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        LOG_ERROR("Unsupported layout transition");
        return;
    }

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = 0;
    barrier.srcAccessMask = src_access_mask;
    barrier.dstAccessMask = dst_access_mask;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = context->device.queues.graphics.index;
    barrier.dstQueueFamilyIndex = context->device.queues.graphics.index;
    barrier.image = image->handle;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(command_buffer->handle, src_stage_mask, dst_stage_mask, 0, 0, 0, 0, 0, 1, &barrier);
}

void vulkan_image_copy_from_buffer(vulkan_context* context, vulkan_command_buffer* command_buffer, vulkan_image* image, VkBuffer buffer)
{
    VkBufferImageCopy copy = {};
    copy.bufferOffset = 0;
    copy.bufferRowLength = 0;
    copy.bufferImageHeight = 0;
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.mipLevel = 0;
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.layerCount = 1;
    copy.imageOffset.x = 0;
    copy.imageOffset.y = 0;
    copy.imageOffset.z = 0;
    copy.imageExtent.width = image->width;
    copy.imageExtent.height = image->height;
    copy.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(command_buffer->handle, buffer, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
}

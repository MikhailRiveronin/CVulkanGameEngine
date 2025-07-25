#include "VulkanImage.h"
#include "VulkanDevice.h"
#include "Core/Logger.h"

void vulkanImageCreate(
    VulkanContext* context,
    VkImageType imageType,
    u32 width, u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memoryProperties,
    b32 createView,
    VkImageAspectFlags aspectFlags,
    VulkanImage* image)
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
    createInfo.mipLevels = 4;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = tiling;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = 0;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VK_CHECK(vkCreateImage(context->device.logicalDevice, &createInfo, context->allocator, &image->handle));

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(context->device.logicalDevice, image->handle, &memoryRequirements);

    i32 memoryType = context->findMemoryType(memoryRequirements.memoryTypeBits, memoryProperties);
    if (memoryType == -1) {
        LOG_ERROR("Required memory type not found. Image not valid");
    }

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = NULL;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = memoryType;
    VK_CHECK(vkAllocateMemory(context->device.logicalDevice, &allocateInfo, context->allocator, &image->memory));
    VK_CHECK(vkBindImageMemory(context->device.logicalDevice, image->handle, image->memory, 0));

    if (createView) {
        image->view = VK_NULL_HANDLE;
        vulkanImageViewCreate(context, format, image, aspectFlags);
    }
}

void vulkanImageViewCreate(
    VulkanContext* context,
    VkFormat format,
    VulkanImage* image,
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
    VK_CHECK(vkCreateImageView(context->device.logicalDevice, &createInfo, context->allocator, &image->view));
}

void vulkanImageDestroy(VulkanContext* context, VulkanImage* image)
{
    if (image->view) {
        vkDestroyImageView(context->device.logicalDevice, image->view, context->allocator);
        image->view = VK_NULL_HANDLE;
    }
    if (image->memory) {
        vkFreeMemory(context->device.logicalDevice, image->memory, context->allocator);
        image->memory = VK_NULL_HANDLE;
    }
    if (image->handle) {
        vkDestroyImage(context->device.logicalDevice, image->handle, context->allocator);
        image->handle = VK_NULL_HANDLE;
    }
}

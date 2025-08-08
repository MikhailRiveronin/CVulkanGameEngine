#pragma once

#include "vulkan_types.inl"

void vulkanImageCreate(
    vulkan_context* context,
    VkImageType imageType,
    u32 width, u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags,
    b32 createView,
    VkImageAspectFlags aspectFlags,
    VulkanImage* image);

void vulkanImageViewCreate(
    vulkan_context* context,
    VkFormat format,
    VulkanImage* image,
    VkImageAspectFlags aspectFlags);

void vulkanImageDestroy(vulkan_context* context, VulkanImage* image);

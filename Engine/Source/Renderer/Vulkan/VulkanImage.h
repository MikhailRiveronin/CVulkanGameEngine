#pragma once

#include "VulkanTypes.inl"

void vulkanImageCreate(
    VulkanContext* context,
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
    VulkanContext* context,
    VkFormat format,
    VulkanImage* image,
    VkImageAspectFlags aspectFlags);

void vulkanImageDestroy(VulkanContext* context, VulkanImage* image);

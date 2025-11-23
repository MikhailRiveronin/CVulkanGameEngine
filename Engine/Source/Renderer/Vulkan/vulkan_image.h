#pragma once

#include "vulkan_structures.h"

void vulkan_image_create(vulkan_context* context, VkImageType imageType, u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, b32 createView, VkImageAspectFlags aspectFlags, vulkan_image* image);

void vulkan_image_view_create(
    vulkan_context* context,
    VkFormat format,
    vulkan_image* image,
    VkImageAspectFlags aspectFlags);

void vulkan_image_destroy(vulkan_context* context, vulkan_image* image);

void vulkan_image_transition_layout(vulkan_context* context, vulkan_command_buffer* command_buffer, vulkan_image* image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

void vulkan_image_copy_from_buffer(vulkan_context* context, vulkan_command_buffer* command_buffer, vulkan_image* image, VkBuffer buffer);

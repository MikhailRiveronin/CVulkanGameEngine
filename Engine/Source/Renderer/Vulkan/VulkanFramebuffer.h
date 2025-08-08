#pragma once

#include "vulkan_types.inl"

void vulkanFramebufferCreate(
    vulkan_context* context,
    vulkan_renderpass* renderpass,
    i32 width,
    i32 height,
    u32 attachmentCount,
    VkImageView* attachments,
    VulkanFramebuffer* framebuffer);

void vulkanFramebufferDestroy(vulkan_context* context, VulkanFramebuffer* framebuffer);

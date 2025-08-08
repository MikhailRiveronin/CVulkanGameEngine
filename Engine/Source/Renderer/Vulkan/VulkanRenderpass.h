#pragma once

#include "vulkan_types.inl"

void vulkan_renderpass_create(
    vulkan_context* context, 
    vulkan_renderpass* renderpass,
    f32 x, f32 y, f32 w, f32 h,
    f32 r, f32 g, f32 b, f32 a,
    f32 depth,
    u32 stencil);

void vulkanRenderpassDestroy(vulkan_context* context, vulkan_renderpass* renderpass);

void vulkan_renderpass_begin(
    vulkan_command_buffer* commandBuffer,
    vulkan_renderpass* renderpass,
    VkFramebuffer framebuffer);

void vulkan_renderpass_end(vulkan_command_buffer* commandBuffer, vulkan_renderpass* renderpass);

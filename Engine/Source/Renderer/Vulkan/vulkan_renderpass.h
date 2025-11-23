#pragma once

#include "vulkan_structures.h"

typedef enum Renderpass_Clear_Flags
{
    RENDERPASS_CLEAR_NONE_FLAG = 0x0,
    RENDERPASS_CLEAR_COLOUR_BUFFER_FLAG = 0x1,
    RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG = 0x2,
    RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG = 0x4
} Renderpass_Clear_Flags;

void vulkan_renderpass_create(vulkan_context const* const context, vec4s render_area, vec4s clear_color, f32 clear_depth, u32 clear_stencil, u8 clear_flags, b8 has_prev_pass, b8 has_next_pass, vulkan_renderpass* const renderpass);

void vulkan_renderpass_destroy(vulkan_context* context, vulkan_renderpass* renderpass);

void vulkan_renderpass_begin(
    vulkan_command_buffer* commandBuffer,
    vulkan_renderpass* renderpass,
    VkFramebuffer framebuffer);

void vulkan_renderpass_end(vulkan_command_buffer* commandBuffer, vulkan_renderpass* renderpass);

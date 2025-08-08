#pragma once

#include "vulkan_types.inl"

void vulkanCommandBufferAllocate(
    vulkan_context* context,
    VkCommandPool pool,
    b8 primary,
    vulkan_command_buffer* commandBuffer);

void vulkanCommandBufferFree(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* commandBuffer);

void vulkan_command_buffer_begin(
    vulkan_command_buffer* commandBuffer,
    b8 singleUse,
    b8 renderpassContinue,
    b8 simultaneousUse);

void vulkan_command_buffer_end(vulkan_command_buffer* commandBuffer);

void vulkanCommandBufferUpdateSubmitted(vulkan_command_buffer* commandBuffer);

void vulkan_command_buffer_reset(vulkan_command_buffer* commandBuffer);

void vulkan_command_buffer_allocate_and_begin_single_use(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* commandBuffer);

void vulkan_command_buffer_end_single_use(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* commandBuffer,
    VkQueue queue);

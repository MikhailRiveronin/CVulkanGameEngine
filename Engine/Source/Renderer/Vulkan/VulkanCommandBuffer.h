#pragma once

#include "VulkanTypes.inl"

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

void vulkanCommandBufferAllocateAndBeginSingleUse(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* commandBuffer);

void vulkanCommandBufferEndSingleUse(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* commandBuffer,
    VkQueue queue);

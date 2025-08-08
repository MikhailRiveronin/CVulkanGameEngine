#pragma once

#include "vulkan_types.inl"

b8 vulkanSwapchainCreate(vulkan_context* context, u32 width, u32 height, VulkanSwapchain* swapchain);
b8 vulkan_swapchain_recreate(vulkan_context* context, u32 width, u32 height, VulkanSwapchain* swapchain);
void vulkanSwapchainDestroy(vulkan_context* context, VulkanSwapchain* swapchain);

b8 vulkan_swapchain_acquire_next_image(
    vulkan_context* context,
    VulkanSwapchain* swapchain,
    u64 timeoutNs,
    VkSemaphore imageAvailableSemaphore,
    VkFence fence,
    u32* imageIndex);

void vulkan_swapchain_present(
    vulkan_context* context,
    VulkanSwapchain* swapchain,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkSemaphore renderCompleteSemaphore,
    u32 presentImageIndex);

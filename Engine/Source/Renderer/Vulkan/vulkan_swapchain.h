#pragma once

#include "vulkan_structures.h"

b8 vulkan_swapchain_create(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* swapchain);
b8 vulkan_swapchain_recreate(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* swapchain);
void vulkanSwapchainDestroy(vulkan_context* context, vulkan_swapchain* swapchain);

b8 vulkan_swapchain_acquire_next_image(
    vulkan_context* context,
    vulkan_swapchain* swapchain,
    u64 timeoutNs,
    VkSemaphore imageAvailableSemaphore,
    VkFence fence,
    u32* imageIndex);

void vulkan_swapchain_present(
    vulkan_context* context,
    vulkan_swapchain* swapchain,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkSemaphore renderCompleteSemaphore,
    u32 presentImageIndex);

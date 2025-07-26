#pragma once

#include "VulkanTypes.inl"

b8 vulkanSwapchainCreate(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain);
b8 vulkanSwapchainRecreate(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain);
void vulkanSwapchainDestroy(VulkanContext* context, VulkanSwapchain* swapchain);

b8 vulkanSwapchainAcquireNextImageIndex(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    u64 timeoutNs,
    VkSemaphore imageAvailableSemaphore,
    VkFence fence,
    u32* imageIndex);

void vulkanSwapchainPresent(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkSemaphore renderCompleteSemaphore,
    u32 presentImageIndex);

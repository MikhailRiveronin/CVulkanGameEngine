#pragma once

#include "defines.h"
#include "VulkanTypes.inl"
#include "Platform/Platform.h"

b8 vulkanDeviceCreate(vulkan_context* context);
void vulkanDeviceDestroy(vulkan_context* context);

void vulkan_device_query_swapchain_support(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VulkanSwapchainSupportInfo* swapchainSupportInfo);

b8 vulkan_device_detect_depth_format(vulkan_device* device);

#pragma once

#include "defines.h"
#include "vulkan_structures.h"
#include "platform/platform.h"

b8 vulkan_device_create(vulkan_context* context);
void vulkanDeviceDestroy(vulkan_context* context);

void vulkan_device_query_swapchain_support(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VulkanSwapchainSupportInfo* swapchainSupportInfo);

b8 vulkan_device_detect_depth_format(vulkan_device* device);

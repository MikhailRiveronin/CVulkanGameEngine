#pragma once

#include "Defines.h"
#include "VulkanTypes.inl"
#include "Platform/Platform.h"

b8 vulkanDeviceCreate(VulkanContext* context);
void vulkanDeviceDestroy(VulkanContext* context);

void vulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VulkanSwapchainSupportInfo* swapchainSupportInfo);

b8 vulkanDeviceDetectDepthFormat(VulkanDevice* device);

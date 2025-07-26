#pragma once

#include "Defines.h"
#include "VulkanTypes.inl"
#include "Platform/Platform.h"

b8 vulkanDeviceInit(VulkanContext* context);
void vulkanDeviceDestroy(VulkanContext* context);

void vulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VulkanSwapchainSupportInfo* swapchainSupportInfo);

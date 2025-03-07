#pragma once

#include "Defines.h"

#include "Renderer/Vulkan/VulkanTypes.h"

#include "Platform/Platform.h"

bool vulkanCreateDevice(VulkanState* state);
void vulkanDestroyDevice(VulkanState* state);

void vulkanDeviceQuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VulkanSwapchainSupportInfo* pOutSwapchainSupportInfo);

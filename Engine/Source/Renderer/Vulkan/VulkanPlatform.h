#pragma once

#include "VulkanTypes.inl"
#include "Containers/DArray.h"

struct PlatformState;
struct VulkanContext;

void vulkanPlatformGetRequiredExtensions(DARRAY_CSTRING* extensions);
b8 platformCreateVulkanSurface(struct PlatformState* platformState, struct VulkanContext* context);

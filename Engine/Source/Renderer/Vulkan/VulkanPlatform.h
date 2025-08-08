#pragma once

#include "vulkan_types.inl"
#include "Containers/darray.h"

struct PlatformState;
struct vulkan_context;

void vulkanPlatformGetRequiredExtensions(DARRAY_CSTRING* extensions);
b8 platformCreateVulkanSurface(struct PlatformState* platformState, struct vulkan_context* context);

#pragma once

#include "vulkan_types.inl"
#include "containers/darray.h"

struct platform_state;
struct vulkan_context;

void vulkanPlatformGetRequiredExtensions(DARRAY_CSTRING* extensions);
b8 platformCreateVulkanSurface(struct platform_state* platformState, struct vulkan_context* context);

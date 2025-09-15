#pragma once

#include "vulkan_types.h"
#include "containers/darray.h"

struct vulkan_context;

void vulkan_platform_get_required_extensions(DARRAY_CSTRING* extensions);

b8 platform_create_vulkan_surface(struct vulkan_context* context);

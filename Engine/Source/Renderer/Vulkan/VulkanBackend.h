#pragma once

#include "VulkanTypes.inl"
#include "Renderer/RendererBackend.h"

b8 vulkanBackendInit(struct renderer_backend* backend, char const* appName, struct PlatformState* platformState);
void vulkanBackendDestroy(struct renderer_backend* backend);

b8 vulkan_backend_begin_frame(renderer_backend* backend, f64 deltaTime);
b8 vulkan_backend_end_frame(struct renderer_backend* backend, f64 deltaTime);

void vulkan_backend_on_resize(renderer_backend* backend, i16 width, i16 height);

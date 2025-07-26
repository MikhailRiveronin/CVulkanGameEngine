#pragma once

#include "VulkanTypes.inl"
#include "Renderer/RendererBackend.h"

b8 vulkanBackendInit(struct RendererBackend* backend, char const* appName, struct PlatformState* platformState);
void vulkanBackendDestroy(struct RendererBackend* backend);

b8 vulkanBackendBeginFrame(struct RendererBackend* backend, f64 deltaTime);
b8 vulkanBackendEndFrame(struct RendererBackend* backend, f64 deltaTime);

void vulkanBackendResize(struct RendererBackend* backend, u16 width, u16 height);

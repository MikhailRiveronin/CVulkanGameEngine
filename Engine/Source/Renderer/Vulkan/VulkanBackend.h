#pragma once

#include "Renderer/RendererBackend.h"

bool vulkanBackendInitialize(struct RendererBackend* backend, const char* appName, struct PlatformState* platformState);
void vulkanBackendTerminate(struct RendererBackend* backend);

bool vulkanBackendBeginFrame(struct RendererBackend* backend, float deltaTime);
bool vulkanBackendEndFrame(struct RendererBackend* backend, float deltaTime);

void vulkanBackendResize(struct RendererBackend* backend, uint16 width, uint16 height);

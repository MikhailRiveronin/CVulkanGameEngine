#pragma once

#include "RendererTypes.inl"

struct PlatformState;

b8 rendererBackendInit(RendererBackendType type, struct PlatformState* platformState, RendererBackend* backend);
void rendererBackendDestroy(RendererBackend* backend);

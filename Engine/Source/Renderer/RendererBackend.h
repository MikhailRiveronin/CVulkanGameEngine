#pragma once

#include "RendererTypes.inl"

struct PlatformState;

b8 renderer_backend_init(RendererBackendType type, struct PlatformState* platformState, renderer_backend* backend);
void rendererBackendDestroy(renderer_backend* backend);

#pragma once

#include "renderer_types.inl"

struct platform_state;

b8 renderer_backend_init(RendererBackendType type, struct platform_state* platformState, renderer_backend* backend);
void rendererBackendDestroy(renderer_backend* backend);

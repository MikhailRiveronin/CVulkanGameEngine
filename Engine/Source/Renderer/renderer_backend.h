#pragma once

#include "renderer_types.inl"

b8 renderer_backend_init(RendererBackendType type, renderer_backend* backend);
void renderer_backend_destroy(renderer_backend* backend);

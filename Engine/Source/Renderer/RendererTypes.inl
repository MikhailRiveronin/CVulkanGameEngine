#pragma once

#include "Defines.h"

typedef enum RendererBackendType {
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} RendererBackendType;

struct PlatformState;

typedef struct renderer_backend {
    struct PlatformState* platformState;
    u64 frameCount;

    b8 (* on_init)(struct renderer_backend* backend, char const* appName, struct PlatformState* platformState);
    void (* on_destroy)(struct renderer_backend* backend);

    b8 (* begin_frame)(struct renderer_backend* backend, f64 deltaTime);
    b8 (* endFrame)(struct renderer_backend* backend, f64 deltaTime);
    void (* on_resize)(struct renderer_backend* backend, i16 width, i16 height);
} renderer_backend;

typedef struct RenderPacket {
    f64 deltaTime;
} RenderPacket;

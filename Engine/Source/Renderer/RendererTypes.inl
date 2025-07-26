#pragma once

#include "Defines.h"

typedef enum RendererBackendType {
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} RendererBackendType;

struct PlatformState;

typedef struct RendererBackend {
    struct PlatformState* platformState;
    u64 frameCount;

    b8 (* init)(struct RendererBackend* backend, char const* appName, struct PlatformState* platformState);
    void (* destroy)(struct RendererBackend* backend);
    b8 (* beginFrame)(struct RendererBackend* backend, f64 deltaTime);
    b8 (* endFrame)(struct RendererBackend* backend, f64 deltaTime);
    void (* resize)(struct RendererBackend* backend, u16 width, u16 height);
} RendererBackend;

typedef struct RenderPacket {
    f64 deltaTime;
} RenderPacket;

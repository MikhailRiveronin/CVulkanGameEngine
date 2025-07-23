#pragma once

#include "Defines.h"

typedef enum
{
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} RendererBackendType;

struct PlatformState;

typedef struct RendererBackend
{
    struct PlatformState* platformState;
    uint64 frameCount;

    bool (* initialize)(struct RendererBackend* backend, const char* appName, struct PlatformState* platformState);
    void (* terminate)(struct RendererBackend* backend);

    bool (* beginFrame)(struct RendererBackend* backend, float deltaTime);
    bool (* endFrame)(struct RendererBackend* backend, float deltaTime);

    void (* resize)(struct RendererBackend* backend, uint16 width, uint16 height);
} RendererBackend;

typedef struct
{
    float deltaTime;
} RenderPacket;

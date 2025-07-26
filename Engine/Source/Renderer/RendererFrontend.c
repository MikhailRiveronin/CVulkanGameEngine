#include "RendererFrontend.h"
#include "RendererBackend.h"
#include "Core/Logger.h"
#include "Core/Memory.h"

static RendererBackend* backend;

b8 rendererInit(char const* appName, struct PlatformState* platformState)
{
    backend = memoryAllocate(sizeof(*backend), MEMORY_TAG_RENDERER);
    rendererBackendInit(RENDERER_BACKEND_TYPE_VULKAN, platformState, backend);
    backend->frameCount = 0;
    if (!backend->init(backend, appName, platformState)) {
        LOG_FATAL("Failed to initialize renderer backend. Shutting down");
        return FALSE;
    }
    return TRUE;
}

void rendererDestroy()
{
    backend->destroy(backend);
    memoryFree(backend, sizeof(*backend), MEMORY_TAG_RENDERER);
}

static b8 rendererBeginFrame(float deltaTime);
static b8 rendererEndFrame(float deltaTime);

b8 rendererDrawFrame(RenderPacket* packet)
{
    if (rendererBeginFrame(packet->deltaTime)) {
        b8 result = rendererEndFrame(packet->deltaTime);
        if (!result) {
            LOG_FATAL("Failed to end frame (rendererEndFrame). Shutting down");
            return FALSE;
        }
    }
    return TRUE;
}

b8 rendererBeginFrame(float deltaTime)
{
    return backend->beginFrame(backend, deltaTime);
}

b8 rendererEndFrame(float deltaTime)
{
    b8 result = backend->endFrame(backend, deltaTime);
    backend->frameCount++;
    return result;
}

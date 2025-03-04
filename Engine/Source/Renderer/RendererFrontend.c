#include "Renderer/RendererFrontend.h"

#include "Core/Logger.h"
#include "Core/Memory.h"

#include "Renderer/RendererBackend.h"

static RendererBackend* backend;

bool rendererStartup(const char* appName, struct PlatformState* platformState)
{
    backend = memoryAllocate(sizeof(*backend), MEMORY_TAG_RENDERER);

    rendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, platformState, backend);
    backend->frameCount = 0;

    if (!backend->initialize(backend, appName, platformState))
    {
        LOG_FATAL("Failed to initialize renderer backend. Shutting down");
        return FALSE;
    }

    return TRUE;
}

void rendererShutdown()
{
    backend->terminate(backend);

    memoryFree(backend, sizeof(*backend), MEMORY_TAG_RENDERER);
}

static bool rendererBeginFrame(float deltaTime);
static bool rendererEndFrame(float deltaTime);

bool rendererDrawFrame(RenderPacket* packet)
{
    if (rendererBeginFrame(packet->deltaTime))
    {
        bool result = rendererEndFrame(packet->deltaTime);
        if (!result)
        {
            LOG_ERROR("Failed to end frame (rendererEndFrame). Shutting down");

            return FALSE;
        }
    }

    return TRUE;
}

bool rendererBeginFrame(float deltaTime)
{
    return backend->beginFrame(backend, deltaTime);
}

bool rendererEndFrame(float deltaTime)
{
    bool result = backend->endFrame(backend, deltaTime);
    backend->frameCount++;

    return result;
}

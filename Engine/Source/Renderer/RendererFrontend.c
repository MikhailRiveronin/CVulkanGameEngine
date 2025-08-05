#include "RendererFrontend.h"
#include "RendererBackend.h"
#include "core/logger.h"
#include "Core/memory.h"

static renderer_backend* backend;

static b8 renderer_begin_frame(float deltaTime);
static b8 rendererEndFrame(float deltaTime);

b8 rendererInit(char const* appName, struct PlatformState* platformState)
{
    backend = memory_allocate(sizeof(*backend), MEMORY_TAG_RENDERER);
    renderer_backend_init(RENDERER_BACKEND_TYPE_VULKAN, platformState, backend);
    backend->frameCount = 0;
    if (!backend->on_init(backend, appName, platformState)) {
        LOG_FATAL("Failed to initialize renderer backend. Shutting down");
        return FALSE;
    }
    return TRUE;
}

void rendererDestroy()
{
    backend->on_destroy(backend);
    memory_free(backend, sizeof(*backend), MEMORY_TAG_RENDERER);
}

b8 renderer_draw_frame(RenderPacket* packet)
{
    if (renderer_begin_frame(packet->deltaTime)) {
        b8 result = rendererEndFrame(packet->deltaTime);
        if (!result) {
            LOG_FATAL("Failed to end frame (rendererEndFrame). Shutting down");
            return FALSE;
        }
    }
    return TRUE;
}

void renderer_frontend_resize(i16 width, i16 height)
{
    if (backend) {
        backend->on_resize(backend, width, height);
    } else {
        LOG_ERROR("renderer_frontend_resize: backend is NULL");
    }
}

b8 renderer_begin_frame(float deltaTime)
{
    return backend->begin_frame(backend, deltaTime);
}

b8 rendererEndFrame(float deltaTime)
{
    b8 result = backend->endFrame(backend, deltaTime);
    backend->frameCount++;
    return result;
}

#include "RendererBackend.h"
#include "Vulkan/VulkanBackend.h"
#include "Core/Memory.h"

b8 rendererBackendInit(RendererBackendType type, struct PlatformState* platformState, RendererBackend* backend)
{
    backend->platformState = platformState;
    switch (type) {
        case RENDERER_BACKEND_TYPE_VULKAN:
            backend->init = vulkanBackendInit;
            backend->destroy = vulkanBackendDestroy;
            backend->beginFrame = vulkanBackendBeginFrame;
            backend->endFrame = vulkanBackendEndFrame;
            backend->resize = vulkanBackendResize;
            return TRUE;
    }
    return FALSE;
}

void rendererBackendDestroy(RendererBackend* backend)
{
    backend->init = NULL;
    backend->destroy = NULL;
    backend->beginFrame = NULL;
    backend->endFrame = NULL;
    backend->resize = NULL;
}

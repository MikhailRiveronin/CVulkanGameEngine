#include "Renderer/RendererBackend.h"

#include "Renderer/Vulkan/VulkanBackend.h"

#include "Core/Memory.h"

bool rendererBackendCreate(RendererBackendType type, struct PlatformState* platformState, RendererBackend* outBackend)
{
    outBackend->platformState = platformState;

    if (type == RENDERER_BACKEND_TYPE_VULKAN)
    {
        outBackend->initialize = vulkanBackendInitialize;
        outBackend->terminate = vulkanBackendTerminate;
        outBackend->beginFrame = vulkanBackendBeginFrame;
        outBackend->endFrame = vulkanBackendEndFrame;
        outBackend->resize = vulkanBackendResize;

        return TRUE;
    }

    return FALSE;
}

void rendererBackendDestroy(RendererBackend* backend)
{
    memoryZero(backend, sizeof(*backend));
}

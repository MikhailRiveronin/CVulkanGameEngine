#include "renderer_backend.h"
#include "Vulkan/vulkan_backend.h"
#include "core/memory.h"

b8 renderer_backend_init(RendererBackendType type, renderer_backend* backend)
{
    switch (type) {
        case RENDERER_BACKEND_TYPE_VULKAN:
            backend->on_init = vulkan_backend_create;
            backend->on_destroy = vulkan_backend_destroy;
            backend->begin_frame = vulkan_backend_begin_frame;
            backend->on_update_global_state = vulkan_backend_update_global_state;
            backend->endFrame = vulkan_backend_end_frame;
            backend->on_resize = vulkan_backend_on_resize;
            backend->on_update_object_state = vulkan_backend_update_object_state;
            backend->frameCount = 0;
            return TRUE;
    }
    return FALSE;
}

void renderer_backend_destroy(renderer_backend* backend)
{
    backend->on_init = NULL;
    backend->on_destroy = NULL;
    backend->begin_frame = NULL;
    backend->on_update_global_state = 0;
    backend->endFrame = NULL;
    backend->on_resize = NULL;
    backend->on_update_object_state = 0;
}

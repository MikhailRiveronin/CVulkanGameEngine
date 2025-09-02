#include "renderer_backend.h"
#include "vulkan/vulkan_backend.h"
#include "core/memory_utils.h"

b8 renderer_backend_create(renderer_backend_type type, renderer_backend* backend)
{
    switch (type) {
        case RENDERER_BACKEND_TYPE_VULKAN:
            backend->init = vulkan_backend_create;
            backend->destroy = vulkan_backend_destroy;
            backend->begin_frame = vulkan_backend_begin_frame;
            backend->update_global_state = vulkan_backend_update_global_state;
            backend->update_global_ui_state = vulkan_renderer_update_global_ui_state;
            backend->end_frame = vulkan_backend_end_frame;
            backend->resize = vulkan_backend_on_resize;
            backend->draw_geometry = vulkan_backend_draw_geometry;
            backend->create_texture = vulkan_backend_create_texture;
            backend->destroy_texture = vulkan_backend_destroy_texture;
            backend->create_material = vulkan_backend_create_material;
            backend->destroy_material = vulkan_backend_destroy_material;
            backend->create_geometry = vulkan_backend_create_geometry;
            backend->destroy_geometry = vulkan_backend_destroy_geometry;
            backend->begin_renderpass = vulkan_renderer_begin_renderpass;
            backend->end_renderpass = vulkan_renderer_end_renderpass;
            backend->frameCount = 0;
            return TRUE;
    }
    return FALSE;
}

void renderer_backend_destroy(renderer_backend* backend)
{
    backend->init = NULL;
    backend->destroy = NULL;
    backend->begin_frame = NULL;
    backend->update_global_state = 0;
    backend->end_frame = NULL;
    backend->resize = NULL;
    backend->draw_geometry = 0;
    backend->create_texture = 0;
    backend->destroy_texture = 0;
    backend->create_material = 0;
    backend->destroy_material = 0;
    backend->create_geometry = 0;
    backend->destroy_geometry = 0;
    backend->update_global_ui_state = 0;
    backend->begin_renderpass = 0;
    backend->end_renderpass = 0;
}

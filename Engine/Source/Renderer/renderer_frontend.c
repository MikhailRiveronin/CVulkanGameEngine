#include "renderer_frontend.h"

#include "renderer_backend.h"
#include "core/logger.h"
#include "core/memory_utils.h"
#include "core/string_utils.h"
#include "core/events.h"
#include "math/kmath.h"
#include "systems/texture_system.h"
#include "systems/material_system.h"

typedef struct renderer_system_state {
    renderer_backend backend;

    mat4 proj;
    mat4 view;

    mat4 ui_projection;
    mat4 ui_view;

    f32 near;
    f32 far;
} renderer_system_state;

static renderer_system_state* system_state;

static b8 renderer_begin_frame(float deltaTime);
static b8 rendererEndFrame(float deltaTime);

b8 renderer_system_startup(u64* memory_size, void* memory, char const* app_name)
{
    *memory_size = sizeof(*system_state);
    if (!memory) {
        return TRUE;
    }

    system_state = memory;

    renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, &system_state->backend);

    if (!system_state->backend.init(&system_state->backend, app_name)) {
        LOG_FATAL("Failed to initialize renderer system_state->backend. Shutting down");
        return FALSE;
    }

    system_state->near = 0.1f;
    system_state->far = 1000.f;
    system_state->proj = mat4_perspective(deg_to_rad(45.f), 1280.f / 720.f, system_state->near, system_state->far);

    system_state->view = mat4_translation((vec3){30, 0, 0.0f});
    system_state->view = mat4_inverse(system_state->view);

    // UI projection/view
    system_state->ui_projection = mat4_orthographic(0, 1280.0f, 720.0f, 0, -100.f, 100.0f);  // Intentionally flipped on y axis.
    system_state->ui_view = mat4_inverse(mat4_identity());

    return TRUE;
}

void renderer_system_shutdown()
{
    if (system_state) {
        system_state->backend.destroy(&system_state->backend);
        system_state = 0;
    }
}

b8 renderer_frontend_draw_frame(render_packet* packet)
{
    if (renderer_begin_frame(packet->delta_time)) {
        // World renderpass
        if (!system_state->backend.begin_renderpass(&system_state->backend, BUILTIN_RENDERPASS_WORLD)) {
            LOG_ERROR("backend.begin_renderpass -> BUILTIN_RENDERPASS_WORLD failed. Application shutting down...");
            return FALSE;
        }

        system_state->backend.update_global_state(system_state->view, system_state->proj, vec3_zero(), vec4_one(), 0);

        u32 count = packet->geometry_count;
        for (u32 i = 0; i < count; ++i) {
            system_state->backend.draw_geometry(packet->geometries[i]);
        }

        if (!system_state->backend.end_renderpass(&system_state->backend, BUILTIN_RENDERPASS_WORLD)) {
            LOG_ERROR("backend.end_renderpass -> BUILTIN_RENDERPASS_WORLD failed. Application shutting down...");
            return FALSE;
        }
        // End world renderpass

        // UI renderpass
        if (!system_state->backend.begin_renderpass(&system_state->backend, BUILTIN_RENDERPASS_UI)) {
            LOG_ERROR("backend.begin_renderpass -> BUILTIN_RENDERPASS_UI failed. Application shutting down...");
            return FALSE;
        }

        // Update UI global state
        system_state->backend.update_global_ui_state(system_state->ui_projection, system_state->ui_view, 0);

        // Draw ui geometries.
        count = packet->ui_geometry_count;
        for (u32 i = 0; i < count; ++i) {
            system_state->backend.draw_geometry(packet->ui_geometries[i]);
        }

        if (!system_state->backend.end_renderpass(&system_state->backend, BUILTIN_RENDERPASS_UI)) {
            LOG_ERROR("backend.end_renderpass -> BUILTIN_RENDERPASS_UI failed. Application shutting down...");
            return FALSE;
        }
        // End UI renderpass

        b8 result = rendererEndFrame(packet->delta_time);
        if (!result) {
            LOG_FATAL("Failed to end frame (rendererEndFrame). Shutting down");
            return FALSE;
        }
    }
    return TRUE;
}

void renderer_frontend_resize(i16 width, i16 height)
{
    if (system_state) {
        system_state->proj = mat4_perspective(
            deg_to_rad(45.f),
            width / height,
            system_state->near,
            system_state->far);
        system_state->ui_projection = mat4_orthographic(0, (f32)width, (f32)height, 0, -100.f, 100.0f); // Intentionally flipped on y axis.
        system_state->backend.resize(&system_state->backend, width, height);
    } else {
        LOG_ERROR("renderer_frontend_resize: system_state->backend is NULL");
    }
}

b8 renderer_begin_frame(float deltaTime)
{
    return system_state->backend.begin_frame(&system_state->backend, deltaTime);
}

b8 rendererEndFrame(float deltaTime)
{
    b8 result = system_state->backend.end_frame(&system_state->backend, deltaTime);
    system_state->backend.frameCount++;
    return result;
}

void renderer_frontend_create_texture(u8 const* pixels, texture* texture)
{
    system_state->backend.create_texture(pixels, texture);
}

void renderer_frontend_destroy_texture(texture* texture)
{
    system_state->backend.destroy_texture(texture);
}

void renderer_frontend_set_view(mat4 view)
{
    system_state->view = view;
}

b8 renderer_frontend_create_material(material* material)
{
    return system_state->backend.create_material(material);
}

void renderer_frontend_destroy_material(material* material)
{
    system_state->backend.destroy_material(material);
}

b8 renderer_create_geometry(geometry* geometry, u32 vertex_size, u32 vertex_count, void const* vertices, u32 index_size, u32 index_count, u32 const* indices)
{
    return system_state->backend.create_geometry(geometry, vertex_size, vertex_count, vertices, index_size, index_count, indices);
}

void renderer_destroy_geometry(geometry* geometry)
{
    system_state->backend.destroy_geometry(geometry);
}

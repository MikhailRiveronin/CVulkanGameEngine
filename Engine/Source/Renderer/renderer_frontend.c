#include "renderer_frontend.h"

#include "renderer_backend.h"
#include "core/logger.h"
#include "core/memory_utils.h"
#include "core/string_utils.h"
#include "math/kmath.h"

typedef struct renderer_system_state {
    renderer_backend backend;

    mat4 proj;
    mat4 view;

    f32 near;
    f32 far;

    texture default_texture;
    texture test_diffuse;
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

    if (!system_state->backend.on_init(&system_state->backend, app_name)) {
        LOG_FATAL("Failed to initialize renderer system_state->backend. Shutting down");
        return FALSE;
    }

    // NOTE: Create default texture, a 256x256 blue/white checkerboard pattern.
    // This is done in code to eliminate asset dependencies.
    LOG_TRACE("Creating default texture...");
    const u32 tex_dimension = 256;
    const u32 channels = 4;
    const u32 pixel_count = tex_dimension * tex_dimension;
    u8 pixels[256 * 256 * 4];
    //u8* pixels = kallocate(sizeof(u8) * pixel_count * bpp, MEMORY_TAG_TEXTURE);
    memory_set(pixels, 255, sizeof(u8) * pixel_count * channels);

    // Each pixel.
    for (u64 row = 0; row < tex_dimension; ++row) {
        for (u64 col = 0; col < tex_dimension; ++col) {
            u64 index = (row * tex_dimension) + col;
            u64 index_bpp = index * channels;
            if (row % 2) {
                if (col % 2) {
                    pixels[index_bpp + 0] = 0;
                    pixels[index_bpp + 1] = 0;
                }
            } else {
                if (!(col % 2)) {
                    pixels[index_bpp + 0] = 0;
                    pixels[index_bpp + 1] = 0;
                }
            }
        }
    }
    renderer_frontend_create_texture(pixels, &system_state->default_texture);

    system_state->near = 0.1f;
    system_state->far = 1000.f;
    system_state->proj = mat4_perspective(deg_to_rad(45.f), 1280.f / 720.f, system_state->near, system_state->far);

    system_state->view = mat4_translation((vec3){0, 0, -30.0f});
    system_state->view = mat4_inverse(system_state->view);

    return TRUE;
}

void renderer_system_shutdown()
{
    renderer_frontend_destroy_texture(&system_state->default_texture);
    renderer_frontend_destroy_texture(&system_state->test_diffuse);
    system_state->backend.on_destroy(&system_state->backend);
    system_state = 0;
}

b8 renderer_frontend_draw_frame(render_packet* packet)
{
    if (renderer_begin_frame(packet->deltaTime)) {
        system_state->backend.on_update_global_state(
            system_state->view, system_state->proj,
            vec3_zero(), vec4_one(), 0);

        static f32 angle = 0.01f;
        angle += 0.001f;
        quat rotation = quat_from_axis_angle(vec3_forward(), angle, FALSE);

        geometry_render_data render_data = {};
        render_data.material->internal_id = 0;
        render_data.world = quat_to_rotation_matrix(rotation, vec3_zero());
        render_data.material->diffuse_map.texture = &system_state->test_diffuse;
        system_state->backend.on_update_object_state(render_data);

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
    if (system_state) {
        system_state->proj = mat4_perspective(
            deg_to_rad(45.f),
            width / height,
            system_state->near,
            system_state->far);
        system_state->backend.on_resize(&system_state->backend, width, height);
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
    b8 result = system_state->backend.endFrame(&system_state->backend, deltaTime);
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

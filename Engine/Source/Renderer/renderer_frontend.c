#include "renderer_frontend.h"
#include "renderer_backend.h"
#include "core/logger.h"
#include "core/memory.h"
#include "core/string_utils.h"
#include "math/kmath.h"
#include "resources/resource_types.h"
#include "third_party/stb_image.h"

typedef struct renderer_system_state {
    renderer_backend backend;
    mat4 proj;
    mat4 view;

    texture default_texture;
    texture test_diffuse;
} renderer_system_state;
static renderer_system_state* system_state;

void create_texture(texture* t)
{
    memory_zero(t, sizeof(*t));
    t->generation = INVALID_ID;
}

b8 load_texture(char const* filename, texture* t)
{
    char* format = "assets/textures/%s.%s";
    char absolute_path[512];
    string_format(absolute_path, format, filename, "png");

    texture temp;

    i32 desired_channel_count = 4;
    u8* data = stbi_load(
        absolute_path,
        (i32*)&temp.widht, (i32*)&temp.height,
        (i32*)&temp.channel_count, desired_channel_count);
    temp.channel_count = desired_channel_count;

    if (data) {
        u32 current_generation = t->generation;
        t->generation = INVALID_ID;

        u32 total_size = temp.widht * temp.height * temp.channel_count;

        b8 has_transparency = FALSE;
        for (u32 i = 0; i < total_size; i += desired_channel_count) {
            u8 alpha = data[i + 3];
            if (alpha < 255) {
                has_transparency = TRUE;
                break;
            }
        }

        if (stbi_failure_reason()) {
            LOG_WARNING("Failed to load file '%s': %s", absolute_path, stbi_failure_reason());
        }

        renderer_create_texture(
            filename, TRUE,
            temp.widht, temp.height, temp.channel_count,
            data, has_transparency, &temp);

        texture old = *t;
        *t = temp;
        renderer_destroy_texture(&old);

        if (current_generation == INVALID_ID) {
            t->generation = 0;
        } else {
            t->generation = current_generation + 1;
        }

        stbi_image_free(data);
        return TRUE;
    }

    if (stbi_failure_reason()) {
        LOG_WARNING("Failed to load file '%s': %s", absolute_path, stbi_failure_reason());
    }
    return FALSE;
}

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

    system_state->backend.default_diffuse = &system_state->test_diffuse;

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
    renderer_create_texture(
        "default",
        FALSE,
        tex_dimension,
        tex_dimension,
        4,
        pixels,
        FALSE,
        &system_state->default_texture);

    create_texture(&system_state->test_diffuse);

    return TRUE;
}

void renderer_system_shutdown()
{
    renderer_destroy_texture(&system_state->default_texture);
    renderer_destroy_texture(&system_state->test_diffuse);
    system_state->backend.on_destroy(&system_state->backend);
    system_state = 0;
}

b8 renderer_draw_frame(render_packet* packet)
{
    if (renderer_begin_frame(packet->deltaTime)) {

        system_state->backend.on_update_global_state(mat4_identity(), mat4_identity(), vec3_zero(), vec4_one(), 0);

        static f32 angle = 0.01f;
        angle += 0.001f;
        quat rotation = quat_from_axis_angle(vec3_forward(), angle, FALSE);

        geometry_render_data render_data = {};
        render_data.object_id = 0;
        render_data.world = quat_to_rotation_matrix(rotation, vec3_zero());
        render_data.textures[0] = &system_state->test_diffuse;
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

void renderer_create_texture(
    char const* name,
    b8 auto_release,
    i32 width,
    i32 height,
    i32 channel_count,
    u8 const* pixels,
    b8 has_transparency,
    struct texture* texture)
{
    system_state->backend.create_texture(
        name,
        auto_release,
        width, height,
        channel_count, pixels,
        has_transparency,
        texture);
}

void renderer_destroy_texture(struct texture* texture)
{
    system_state->backend.destroy_texture(texture);
}

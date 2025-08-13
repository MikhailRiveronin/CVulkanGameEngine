#include "renderer_frontend.h"
#include "renderer_backend.h"
#include "core/logger.h"
#include "core/memory.h"
#include "math/kmath.h"

typedef struct renderer_system_state {
    renderer_backend backend;
    mat4 proj;
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

    renderer_backend_init(RENDERER_BACKEND_TYPE_VULKAN, &system_state->backend);

    if (!system_state->backend.on_init(&system_state->backend, app_name)) {
        LOG_FATAL("Failed to initialize renderer system_state->backend. Shutting down");
        return FALSE;
    }
    return TRUE;
}

void renderer_system_shutdown()
{
    system_state->backend.on_destroy(&system_state->backend);
    system_state = 0;
}

b8 renderer_draw_frame(RenderPacket* packet)
{
    if (renderer_begin_frame(packet->deltaTime)) {

        system_state->backend.on_update_global_state(mat4_identity(), mat4_identity(), vec3_zero(), vec4_one(), 0);

        static f32 angle = 0.01f;
        angle += 0.001f;
        quat rotation = quat_from_axis_angle(vec3_forward(), angle, FALSE);
        mat4 world = quat_to_rotation_matrix(rotation, vec3_zero());
        system_state->backend.on_update_object_state(world);

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

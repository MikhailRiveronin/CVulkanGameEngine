#include "game.h"

#include <core/input.h>
#include <core/logger.h>
#include <systems/event_system.h>
#include <systems/memory_system.h>

// TODO: Temp
#include <renderer/renderer_frontend.h>
// TODO: Temp

static void recalculate_view_matrix(game_state* state);
static b8 game_on_event(u16 code, void const* sender, void const* listener, event_context context);

b8 game_init(game_instance* instance)
{
    game_state* state = (game_state*)instance->internal;
    state->delta_time = 0.;
    state->camera.yaw = -1.f * GLM_PI_2f;
    state->camera.pitch = 0.f;
    state->camera.position = (vec3s){ 0.f, 0.f, 30.f };
    state->camera.movement_speed = 50.f;
    state->camera.sensitivity = 0.01f;
    state->camera.dirty = TRUE;
    recalculate_view_matrix(state);

    event_register(EVENT_CODE_BUTTON_PRESSED, instance, game_on_event);
    event_register(EVENT_CODE_MOUSE_MOVED, instance, game_on_event);
    event_register(EVENT_CODE_KEY_PRESSED, instance, game_on_event);

    return TRUE;
}

b8 game_update(game_instance* instance, f64 delta_time)
{
    game_state* state = (game_state*)instance->internal;
    state->delta_time = delta_time;

    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = memory_system_allocation_count();
    if (input_is_key_up('M') && input_was_key_down('M'))
    {
        LOG_DEBUG("game_update: Allocation count: %llu (%llu this frame)", alloc_count, alloc_count - prev_alloc_count);
    }

    if (input_is_key_up('T') && input_was_key_down('T'))
    {
        LOG_DEBUG("game_update: Texture swap");
        event_context context = {};
        event_notify(EVENT_CODE_DEBUG0, instance, context);
    }

    // TODO: Temp
    renderer_frontend_set_view(state->camera.view);
    // TODO: Temp
    return TRUE;
}

b8 game_render(game_instance* instance, f64 delta_time)
{
    return TRUE;
}

void game_resize(game_instance* instance, u32 width, u32 height)
{
}

void recalculate_view_matrix(game_state* state)
{
    if (state->camera.dirty)
    {
        vec3s target = glms_normalize((vec3s){ cosf(state->camera.yaw) * cosf(state->camera.pitch), sinf(state->camera.pitch), sinf(state->camera.yaw) * cosf(state->camera.pitch) });
        vec3s direction = glms_normalize(glms_vec3_add(state->camera.position, target));
        state->camera.view = glms_look(state->camera.position, direction, (vec3s){ 0.f, 1.f, 0.f });
        state->camera.dirty = FALSE;
    }
}

b8 game_on_event(u16 code, void const* sender, void const* listener, event_context context)
{
    game_instance* instance = (game_instance*)listener;
    game_state* state = (game_state*)instance->internal;

    switch (code)
    {
        case EVENT_CODE_BUTTON_PRESSED:
        {
            i16 x = context.as.i16[0];
            i16 y = context.as.i16[1];
            state->camera.last_x = x;
            state->camera.last_y = y;

            return FALSE;
        }

        case EVENT_CODE_MOUSE_MOVED:
        {
            i16 x = context.as.i16[0];
            i16 y = context.as.i16[1];
            i16 offset_x = x - state->camera.last_x;
            i16 offset_y = y - state->camera.last_y;
            state->camera.last_x = x;
            state->camera.last_y = y;
            state->camera.yaw -= (float)offset_x * state->camera.sensitivity;
            state->camera.pitch -= (float)offset_y * state->camera.sensitivity;
            state->camera.pitch = glm_clamp(state->camera.pitch, -89.f, 89.f);
            state->camera.dirty = TRUE;

            return FALSE;
        }

        case EVENT_CODE_KEY_PRESSED:
        {
            u16 key_code = context.as.u16[0];
            switch (key_code)
            {
                case KEY_W:
                {
                    vec3s target = glms_normalize((vec3s){ cosf(state->camera.yaw) * cosf(state->camera.pitch), sinf(state->camera.pitch), sinf(state->camera.yaw) * cosf(state->camera.pitch) });
                    vec3s direction = glms_normalize(glms_vec3_add(state->camera.position, target));
                    state->camera.position = glms_vec3_add(state->camera.position, glms_vec3_scale(direction, state->camera.movement_speed * state->delta_time));
                    state->camera.dirty = TRUE;

                    return FALSE;
                }

                case KEY_S:
                {
                    vec3s target = glms_normalize((vec3s){ cosf(state->camera.yaw) * cosf(state->camera.pitch), sinf(state->camera.pitch), sinf(state->camera.yaw) * cosf(state->camera.pitch) });
                    vec3s direction = glms_normalize(glms_vec3_add(state->camera.position, target));
                    state->camera.position = glms_vec3_add(state->camera.position, glms_vec3_scale(direction, -1.f * state->camera.movement_speed * state->delta_time));
                    state->camera.dirty = TRUE;

                    return FALSE;
                }

                case KEY_A:
                {
                    vec3s target = glms_normalize((vec3s){ cosf(state->camera.yaw) * cosf(state->camera.pitch), sinf(state->camera.pitch), sinf(state->camera.yaw) * cosf(state->camera.pitch) });
                    vec3s right = glms_cross(target, (vec3s){ 0.f, 1.f, 0.f });
                    state->camera.position = glms_vec3_add(state->camera.position, glms_vec3_scale(right, -1.f * state->camera.movement_speed * state->delta_time));
                    state->camera.dirty = TRUE;

                    return FALSE;
                }

                case KEY_D:
                {
                    vec3s target = glms_normalize((vec3s){ cosf(state->camera.yaw) * cosf(state->camera.pitch), sinf(state->camera.pitch), sinf(state->camera.yaw) * cosf(state->camera.pitch) });
                    vec3s right = glms_cross(target, (vec3s){ 0.f, 1.f, 0.f });
                    state->camera.position = glms_vec3_add(state->camera.position, glms_vec3_scale(right, state->camera.movement_speed * delta_time));
                    state->camera.dirty = TRUE;

                    return FALSE;
                }

                default:
                    return FALSE;
            }

            recalculateViewMatrix(state);
            return FALSE;
        }

        default:
            return FALSE;
    }
}

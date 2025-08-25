#include "game.h"

#include <core/input.h>
#include <core/logger.h>
#include <core/memory_utils.h>
#include <core/events.h>
#include <math/kmath.h>

// TODO: Temp
#include <renderer/renderer_frontend.h>

static void recalculate_view_matrix(game_state* state);
static void camera_pitch(game_state* state, f32 amount);
static void camera_yaw(game_state* state, f32 amount);

b8 game_init(game* game)
{
    game_state* state = (game_state*)game->state;
    state->camera.pos = (vec3){ 0.f, 0.f, -30.f };
    state->camera.euler = vec3_zero();
    state->camera.view = mat4_translation(state->camera.pos);
    state->camera.view = mat4_inverse(state->camera.view);
    state->camera.dirty = TRUE;

    return TRUE;
}

b8 game_update(game* game, f64 delta_time)
{
    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = memory_allocation_count();
    if (input_is_key_up('M') && input_was_key_down('M')) {
        LOG_DEBUG("Allocations: %llu (%llu this frame)", alloc_count, alloc_count - prev_alloc_count);
    }

    if (input_is_key_up('T') && input_was_key_down('T')) {
        LOG_DEBUG("Swapping texture!");
        event_context context = {};
        event_notify(EVENT_CODE_DEBUG0, game, context);
    }

    game_state* state = (game_state*)game->state;

    if (input_is_key_down('A') || input_is_key_down(KEY_LEFT)) {
        camera_yaw(state, 1.0f * delta_time);
    }

    if (input_is_key_down('D') || input_is_key_down(KEY_RIGHT)) {
        camera_yaw(state, -1.0f * delta_time);
    }

    if (input_is_key_down(KEY_UP)) {
        camera_pitch(state, 1.0f * delta_time);
    }

    if (input_is_key_down(KEY_DOWN)) {
        camera_pitch(state, -1.0f * delta_time);
    }

    f32 temp_move_speed = 50.0f;
    vec3 velocity = vec3_zero();

    if (input_is_key_down('W')) {
        vec3 forward = mat4_forward(state->camera.view);
        velocity = vec3_add(velocity, forward);
    }

    if (input_is_key_down('S')) {
        vec3 backward = mat4_backward(state->camera.view);
        velocity = vec3_add(velocity, backward);
    }

    if (input_is_key_down('Q')) {
        vec3 left = mat4_left(state->camera.view);
        velocity = vec3_add(velocity, left);
    }

    if (input_is_key_down('E')) {
        vec3 right = mat4_right(state->camera.view);
        velocity = vec3_add(velocity, right);
    }

    if (input_is_key_down(KEY_SPACE)) {
        velocity.y += 1.0f;
    }

    if (input_is_key_down('X')) {
        velocity.y -= 1.0f;
    }

    vec3 z = vec3_zero();
    if (!vec3_compare(z, velocity, 0.0002f)) {
        vec3_normalize(&velocity);
        state->camera.pos.x += velocity.x * temp_move_speed * delta_time;
        state->camera.pos.y += velocity.y * temp_move_speed * delta_time;
        state->camera.pos.z += velocity.z * temp_move_speed * delta_time;
        state->camera.dirty = TRUE;
    }

    recalculate_view_matrix(state);
    renderer_frontend_set_view(state->camera.view);

    return TRUE;
}

b8 game_render(game* game, f64 delta_time)
{
    return TRUE;
}

void game_resize(game* game, u32 width, u32 height)
{
}

void recalculate_view_matrix(game_state* state)
{
    if (state->camera.dirty) {
        mat4 rotation = mat4_euler_xyz(state->camera.euler.x, state->camera.euler.y, state->camera.euler.z);
        mat4 translation = mat4_translation(state->camera.pos);

        state->camera.view = mat4_mul(rotation, translation);
        state->camera.view = mat4_inverse(state->camera.view);
        state->camera.dirty = FALSE;
    }
}

void camera_pitch(game_state* state, f32 amount)
{
    state->camera.euler.x += amount;
    f32 limit = deg_to_rad(89.0f);
    state->camera.euler.x = CLAMP(state->camera.euler.x, -limit, limit);
    state->camera.dirty = TRUE;
}

void camera_yaw(game_state* state, f32 amount)
{
    state->camera.euler.y += amount;
    state->camera.dirty = TRUE;
}

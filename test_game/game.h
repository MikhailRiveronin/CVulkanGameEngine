#pragma once

#include <defines.h>
#include <game_types.h>
#include <math/math_types.h>

typedef struct game_state
{
    struct
    {
        float yaw;
        float pitch;
        vec3s position;
        mat4s view;
        i16 last_x;
        i16 last_y;
        float movement_speed;
        float sensitivity;
        b8 dirty;
    } camera;

    f64 delta_time;
} game_state;

b8 game_init(game_instance* instance);
b8 game_update(game_instance* instance, f64 delta_time);
b8 game_render(game_instance* instance, f64 delta_time);
void game_resize(game_instance* instance, u32 width, u32 height);

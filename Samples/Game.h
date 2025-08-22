#pragma once

#include <game_types.h>
#include <math/math_types.h>

typedef struct game_state {
    f64 delta_time;

    struct {
        mat4 view;
        vec3 pos;
        vec3 euler;
        b8 dirty;
    } camera;
} game_state;

b8 game_init(game* game);
b8 game_update(game* game, f64 delta_time);
b8 game_render(game* game, f64 delta_time);

void game_resize(game* game, u32 width, u32 height);

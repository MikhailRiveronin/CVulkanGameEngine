#pragma once

#include "defines.h"
#include "core/application.h"

typedef struct game_instance
{
    struct
    {
        i16 x;
        i16 y;
        i16 width;
        i16 height;
        char* name;
    } application_config;

    b8 (* init)(struct game_instance* game);
    b8 (* on_update)(struct game_instance* game, f64 deltaTime);
    b8 (* on_render)(struct game_instance* game, f64 deltaTime);
    void (* on_resize)(struct game_instance* game, u32 width, u32 height);

    u64 required_memory;
    void* internal;
    void* application_block;
} game_instance;

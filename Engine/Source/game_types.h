#pragma once

#include "defines.h"
#include "core/application.h"

typedef struct game_instance
{
    b8 (* init)(struct game_instance* game);
    b8 (* update)(struct game_instance* game, f64 deltaTime);
    b8 (* render)(struct game_instance* game, f64 deltaTime);
    void (* resize)(struct game_instance* game, u32 width, u32 height);
    
    // Game-specific state
    void* state;

    void* app_state;
    application_config app_config;
} game_instance;

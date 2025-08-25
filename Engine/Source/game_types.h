#pragma once

#include "defines.h"
#include "core/application.h"

typedef struct game {
    b8 (* init)(struct game* game);
    b8 (* update)(struct game* game, f64 deltaTime);
    b8 (* render)(struct game* game, f64 deltaTime);
    void (* resize)(struct game* game, u32 width, u32 height);
    
    // Game-specific state
    void* state;

    void* app_state;
    application_config app_config;
} game;

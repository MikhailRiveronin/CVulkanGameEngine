#pragma once

#include "defines.h"
#include "core/application.h"

typedef struct game {
    b8 (* onInit)(struct game* game);
    b8 (* onUpdate)(struct game* game, f64 deltaTime);
    b8 (* onRender)(struct game* game, f64 deltaTime);
    void (* onResize)(struct game* game, i32 width, i32 height);
    
    // Game-specific state
    void* state;

    void* app_state;
    application_config app_config;
} game;

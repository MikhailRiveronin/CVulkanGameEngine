#pragma once

#include "defines.h"
#include "core/application.h"

typedef struct game_instance {
    b8 (* onInit)(struct game_instance* game);
    b8 (* onUpdate)(struct game_instance* game, f64 deltaTime);
    b8 (* onRender)(struct game_instance* game, f64 deltaTime);
    void (* onResize)(struct game_instance* game, i32 width, i32 height);
    
    // Game specific state
    void* specific;

    void* app_state;
    application_config app_config;
} game_instance;

#pragma once

#include "defines.h"
#include "Core/Application.h"

typedef struct Game {
    b8 (* onInit)(struct Game* game);
    b8 (* onUpdate)(struct Game* game, f64 deltaTime);
    b8 (* onRender)(struct Game* game, f64 deltaTime);
    void (* onResize)(struct Game* game, i32 width, i32 height);
    
    // Game specific state
    void* specific;

    void* app_state;
    ApplicationConfig appConfig;
} Game;

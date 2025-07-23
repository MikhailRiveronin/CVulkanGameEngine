#pragma once

#include "Defines.h"
#include "Core/Application.h"

typedef struct Game {
    void* specific;
    b8 (* onInit)(struct Game* game);
    b8 (* onUpdate)(struct Game* game, f64 deltaTime);
    b8 (* onRender)(struct Game* game, f64 deltaTime);
    void (* onResize)(struct Game* game, i32 width, i32 height);

    ApplicationConfig appConfig;
} Game;

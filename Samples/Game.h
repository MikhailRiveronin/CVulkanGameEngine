#pragma once

#include "Defines.h"

typedef struct GameState {
    f64 deltaTime;
} GameState;

b8 gameOnInit(struct Game* game);
b8 gameOnUpdate(struct Game* game, f64 deltaTime);
b8 gameOnRender(struct Game* game, f64 deltaTime);
void gameOnResize(struct Game* game, i32 width, i32 height);

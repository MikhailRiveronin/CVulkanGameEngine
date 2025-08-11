#pragma once

#include "defines.h"

typedef struct GameState {
    f64 deltaTime;
} GameState;

b8 gameOnInit(struct game_instance* game);
b8 gameOnUpdate(struct game_instance* game, f64 deltaTime);
b8 gameOnRender(struct game_instance* game, f64 deltaTime);
void gameOnResize(struct game_instance* game, i32 width, i32 height);

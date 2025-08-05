#pragma once

#include "defines.h"
#include "game_types.h"
#include "Core/Application.h"
#include "core/logger.h"

#include <stdlib.h>

extern b8 createGameState(Game* game);

int main(void)
{
    Game game = {};
    if (!createGameState(&game)) {
        LOG_FATAL("Failed to create game state");
        return EXIT_FAILURE;
    }

    if (!game.onInit || !game.onUpdate || !game.onRender || !game.onResize) {
        LOG_FATAL("Game function pointers must be assigned");
        return EXIT_FAILURE;
    }

    if (!application_init(&game)) {
        LOG_FATAL("Failed to init application");
        return EXIT_FAILURE;
    }

    if (!applicationRun()) {
        LOG_FATAL("Application destroyed incorrectly");
        return EXIT_FAILURE;
    }

    return 0;
}

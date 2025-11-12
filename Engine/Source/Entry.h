#pragma once

#include "defines.h"
#include "game_types.h"
#include "core/application.h"
#include "core/logger.h"

#include <stdlib.h>

extern b8 createGameState(game_instance* game);

int main(void)
{
    game_instance game = {};
    if (!createGameState(&game)) {
        LOG_FATAL("Failed to create game state");
        return EXIT_FAILURE;
    }

    if (!game.init || !game.update || !game.render || !game.resize) {
        LOG_FATAL("Game function pointers must be assigned");
        return EXIT_FAILURE;
    }

    if (!application_init(&game)) {
        LOG_FATAL("Failed to init application");
        return EXIT_FAILURE;
    }

    if (!application_run()) {
        LOG_FATAL("Application destroyed incorrectly");
        return EXIT_FAILURE;
    }

    return 0;
}

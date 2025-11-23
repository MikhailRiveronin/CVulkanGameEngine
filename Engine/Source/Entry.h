#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "game_types.h"

#include <stdlib.h>

extern b8 create_game_instance(game_instance* game);

int main(void)
{
    game_instance instance = {};
    if (!create_game_instance(&instance))
    {
        LOG_FATAL("main: Failed to create game instance");
        return EXIT_FAILURE;
    }

    if (!application_initialize(&instance))
    {
        LOG_FATAL("main: Failed to initialize application");
        return EXIT_FAILURE;
    }

    if (!application_run())
    {
        LOG_FATAL("Application destroyed incorrectly");
        return EXIT_FAILURE;
    }

    return 0;
}

#include <entry.h>

#include "game.h"
#include <systems/memory_system.h>

b8 createGameState(game_instance* game)
{
    game->state = memory_allocate(sizeof(game_state), MEMORY_TAG_GAME);

    game->init = game_init;
    game->update = game_update;
    game->render = game_render;
    game->resize = game_resize;

    game->app_config.x = 100;
    game->app_config.y = 100;
    game->app_config.width = 1280;
    game->app_config.height = 720;
    game->app_config.name = "Applic";

    return TRUE;
}

#include <entry.h>

#include "game.h"
#include <core/memory_utils.h>

b8 createGameState(game* game)
{
    game->state = memory_allocate(sizeof(game_state), MEMORY_TAG_GAME);

    game->onInit = game_init;
    game->onUpdate = game_update;
    game->onRender = game_render;
    game->onResize = game_resize;

    game->app_config.x = 100;
    game->app_config.y = 100;
    game->app_config.width = 1280;
    game->app_config.height = 720;
    game->app_config.name = "Applic";

    return TRUE;
}

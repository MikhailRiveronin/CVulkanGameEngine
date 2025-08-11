#include <entry.h>

#include "game.h"
#include <core/memory.h>

b8 createGameState(game_instance* game)
{
    game->specific = memory_allocate(sizeof(GameState), MEMORY_TAG_GAME);

    game->onInit = gameOnInit;
    game->onUpdate = gameOnUpdate;
    game->onRender = gameOnRender;
    game->onResize = gameOnResize;

    game->app_config.x = 100;
    game->app_config.y = 100;
    game->app_config.width = 1280;
    game->app_config.height = 720;
    game->app_config.name = "Applic";

    return TRUE;
}

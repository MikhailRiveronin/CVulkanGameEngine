#include "Game.h"
#include <Entry.h>

b8 createGameState(Game* game)
{
    game->specific = allocate(sizeof(GameState), MEMORY_TAG_GAME);
    memoryPrintUsageStr();

    game->onInit = gameOnInit;
    game->onUpdate = gameOnUpdate;
    game->onRender = gameOnRender;
    game->onResize = gameOnResize;

    game->appConfig.x = 100;
    game->appConfig.y = 100;
    game->appConfig.width = 1280;
    game->appConfig.height = 720;
    game->appConfig.name = "Applic";

    return TRUE;
}

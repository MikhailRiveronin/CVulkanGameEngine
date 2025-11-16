#include "game.h"

#include <entry.h>

b8 create_game_instance(game_instance* instance)
{
    instance->application_config.x = 100;
    instance->application_config.y = 100;
    instance->application_config.width = 1280;
    instance->application_config.height = 720;
    instance->application_config.name = "Game";

    instance->init = game_init;
    instance->on_update = game_update;
    instance->on_render = game_render;
    instance->on_resize = game_resize;

    instance->required_memory = sizeof(game_state);
    instance->internal = 0;
    instance->application_block = 0;

    return TRUE;
}

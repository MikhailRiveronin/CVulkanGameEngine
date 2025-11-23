#pragma once

#include "defines.h"
#include "game_types.h"

LIB_API b8 application_init(game_instance* instance);
LIB_API b8 application_run(void);

void applicationGetFramebufferSize(u32* width, u32* height);

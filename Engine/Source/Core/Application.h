#pragma once

#include "Defines.h"

struct Game;

typedef struct ApplicationConfig {
    i32 x;
    i32 y;
    i32 width;
    i32 height;
    char* name;
} ApplicationConfig;

API b8 applicationInit(struct Game* game);
API b8 applicationRun(void);

void applicationGetFramebufferSize(u32* width, u32* height);

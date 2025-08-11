#pragma once

#include "defines.h"

struct game_instance;

typedef struct application_config {
    i32 x;
    i32 y;
    i32 width;
    i32 height;
    char* name;
} application_config;

API b8 application_init(struct game_instance* game);
API b8 application_run(void);

void applicationGetFramebufferSize(u32* width, u32* height);

#pragma once

#include "resources/resources.h"

typedef struct Texture_System_Configuration
{
    u32 max_texture_count;
} Texture_System_Configuration;

#define DEFAULT_TEXTURE_NAME "default"

b8 texture_system_startup(u64* required_memory, void* block, Texture_System_Configuration config);
void texture_system_shutdown();
Texture* texture_system_acquire(char const* name, b8 auto_release);
void texture_system_release(char const* name);
Texture* texture_system_get_default_texture();

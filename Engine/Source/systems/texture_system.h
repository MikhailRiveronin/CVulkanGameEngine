#pragma once

#include "resources/resource_types.h"

typedef struct texture_system_config {
    u32 max_texture_count;
} texture_system_config;

#define DEFAULT_TEXTURE_NAME "default"

b8 texture_system_startup(u64* state_size_in_bytes, void* memory, texture_system_config config);
void texture_system_shutdown();

texture* texture_system_acquire_texture(char const* name, b8 auto_release);
void texture_system_release_texture(char const* name);

texture* texture_system_get_default_texture();

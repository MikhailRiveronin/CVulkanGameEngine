#pragma once

#include "renderer/renderer_types.inl"

typedef struct texture_system_config {
    u32 max_texture_count;
} texture_system_config;

#define DEFAULT_TEXTURE_NAME "default"

b8 texture_system_startup(u64* required_memory, void* memory, texture_system_config config);
void texture_system_shutdown();

texture* texture_acquire(char const* name, b8 auto_release);
void texture_release(char const* name);

texture* texture_get_default();

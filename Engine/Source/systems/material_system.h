#pragma once

#include "defines.h"

#include "resources/resource_types.h"

#define DEFAULT_MATERIAL_NAME "default"

typedef struct material_system_config {
    u32 max_material_count;
} material_system_config;

typedef struct material_config {
    char name[MATERIAL_NAME_MAX_LENGTH];
    b8 auto_release;
    vec4 diffuse_colour;
    char diffuse_map_name[TEXTURE_NAME_MAX_LENGTH];
} material_config;

b8 material_system_startup(u64* state_size_in_bytes, void* memory, material_system_config config);
void material_system_shutdown();

material* material_system_acquire_material(char const* name);
material* material_system_acquire_from_config(material_config config);
void material_system_release(char const* name);

material* material_system_get_default_material();

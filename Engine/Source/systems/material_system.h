#pragma once

#include "defines.h"

// #include "resources/resources.h"

// #define DEFAULT_MATERIAL_NAME "default"

typedef struct Material_System_Config
{
    u32 max_material_count;
} Material_System_Config;

// typedef struct Material
// {
//     char name[MAX_MATERIAL_NAME_LENGTH];
//     u32 id;
//     u32 backend_id;
//     Material_Type type;
//     vec4s diffuse_color;
//     Texture_Map diffuse_map;
// } Material;

bool material_system_startup(Material_System_Config* config);
void material_system_shutdown();

Material* material_system_acquire(char const* name);
Material* material_system_acquire_from_config(Material_Config config);
void material_system_release(char const* name);

Material* material_system_get_default_material();

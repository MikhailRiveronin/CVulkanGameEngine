#pragma once

#include "defines.h"

// #include "resources/resources.h"

// #define DEFAULT_MATERIAL_NAME "default"

typedef struct Material_System_Config
{
    u32 max_material_count;
} Material_System_Config;

bool material_system_startup(Material_System_Config* config);
void material_system_shutdown();

Material* material_system_acquire(char const* name);
Material* material_system_acquire_from_config(Material_Config config);
void material_system_release(char const* name);

Material* material_system_get_default_material();

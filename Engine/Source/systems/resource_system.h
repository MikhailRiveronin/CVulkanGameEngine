#pragma once

#include "defines.h"
// #include "resources/resource_types.h"
#include "third_party/cglm/struct.h"

typedef struct Resource_System_Config
{
    u32 initial_resource_lookup_table_size;
} Resource_System_Config;

// typedef enum Resource_Type
// {
//     RESOURCE_TYPE_TEXT,
//     RESOURCE_TYPE_BINARY,
//     RESOURCE_TYPE_IMAGE,
//     RESOURCE_TYPE_MATERIAL,
//     RESOURCE_TYPE_SHADER_CONFIG,
//     RESOURCE_TYPE_STATIC_MESH,
//     RESOURCE_TYPE_ENUM_COUNT
// } Resource_Type;

// typedef struct Resource_Reference
// {
//     char const* filename;
//     u32 index; // in the corresponding array
//     u32 reference_count;
//     bool auto_release; // if reference_count reaches zero
//     u32 generation; // ?
// } Resource_Reference;

typedef struct Resource_Data
{
    char const* filename;
    char const* name;
    u32 generation;
    bool auto_release;
    // Resource_Type type;
    u32 size;
    void* data;
} Resource_Data;

typedef struct Resource_Loader
{
    bool (* load)(char const* filename, Resource_Data* resource);
    void (* unload)(Resource_Data* resource);
} Resource_Loader;

typedef struct Material_Config
{
    char name[128];
    vec4s diffuse_color;
    char diffuse_texture_name[128];
} Material_Config;

bool resource_system_startup();
void resource_system_shutdown();


bool resource_system_load(char const* filename, Resource_Data* resource);
void resource_system_unload(Resource_Data* resource);

/**
 * @brief Returns the slot (index) of the resource in the corresponding array.
 */
u32 resource_system_get_reference(char const* name);
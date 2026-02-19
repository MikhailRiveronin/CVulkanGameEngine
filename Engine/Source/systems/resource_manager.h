#pragma once

#include "defines.h"
// #include "resources/resource_types.h"
#include "third_party/cglm/struct.h"

typedef struct Resource_System_Config
{
    u32 initial_resource_lookup_table_size;
} Resource_System_Config;



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
    char name[MAX_MATERIAL_NAME_LENGTH];
    u32 id;
    u32 backend_id;
    Material_Type type;
    vec4s diffuse_color;
    Texture_Map diffuse_map;
} Material_Config;

bool resource_manager_startup();
void resource_manager_shutdown();


bool resource_manager_load(char const* filename, u32 slot, void* resource);
void resource_manager_unload(Resource_Data* resource);

void* resource_manager_acquire(char const* filepath, bool auto_release);

/**
 * @brief Returns the slot (index) of the resource in the corresponding array.
 */
u32 resource_manager_get_reference(char const* name);
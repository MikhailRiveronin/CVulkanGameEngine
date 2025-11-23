#pragma once

#include "defines.h"
#include "resources/resources.h"

typedef struct Resource_System_Config
{
    u32 max_loader_count;
    char* asset_folder_path;
} Resource_System_Config;

typedef struct Resource_Loader
{
    u32 id;
    resource_type type;
    b8 (* load)(char const* const filename, Resource* const resource);
    void (* unload)(Resource* const resource);
} Resource_Loader;

b8 resource_system_startup(u64* const required_memory, void* const block, Resource_System_Config config);
void resource_system_shutdown();
b8 resource_system_register_loader(Resource_Loader loader);
b8 resource_system_load(char const* name, resource_type type, Resource* resource);
void resource_system_unload(Resource* resource);
char const* resource_system_asset_folder();

#pragma once

#include "resources/resource_types.h"

typedef struct resource_system_config {
    u32 max_loader_count;
    char* asset_folder_path;
} resource_system_config;

typedef struct resource_loader {
    u32 id;
    resource_type type;
    char const* type_str;

    b8 (* load)(struct resource_loader* self, char const* name, resource* resource);
    void (* unload)(struct resource_loader* self, resource* resource);
} resource_loader;

b8 resource_system_startup(u64* state_size_in_bytes, void* memory, resource_system_config config);
void resource_system_shutdown();

b8 resource_system_register_loader(resource_loader new_loader);

b8 resource_system_load(char const* name, resource_type type, resource* resource);
void resource_system_unload(resource* resource);

char const* resource_system_asset_folder_path();

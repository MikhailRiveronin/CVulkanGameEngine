#include "resource_system.h"

#include "core/logger.h"
#include "resources/loaders/text_loader.h"
#include "resources/loaders/binary_loader.h"
#include "resources/loaders/image_loader.h"
#include "resources/loaders/material_loader.h"

typedef struct resource_system_state {
    resource_system_config config;
    resource_loader* registered_loaders;
} resource_system_state;

static resource_system_state* system_state;

b8 load(char const* name, resource_loader* loader, resource* resource);

b8 resource_system_startup(u64* state_size_in_bytes, void* memory, resource_system_config config)
{
    if (config.max_loader_count == 0) {
        LOG_FATAL("resource_system_startup: Failed to startup resource system because config.max_loader_count == 0");
        return FALSE;
    }

    u64 state_struct_size_in_bytes = sizeof(*system_state);
    u64 array_size_in_bytes = config.max_loader_count * sizeof(resource_loader);
    *state_size_in_bytes = state_struct_size_in_bytes + array_size_in_bytes;

    if (!memory) {
        return TRUE;
    }

    system_state = memory;
    system_state->config = config;

    void* array_block = (char*)system_state + state_struct_size_in_bytes;
    system_state->registered_loaders = array_block;

    u32 count = config.max_loader_count;
    for (u32 i = 0; i < count; ++i) {
        system_state->registered_loaders[i].id = INVALID_ID;
    }

    resource_system_register_loader(text_resource_loader_create());
    resource_system_register_loader(binary_loader_create());
    resource_system_register_loader(image_loader_create());
    resource_system_register_loader(material_loader_create());

    LOG_INFO("resource_system_startup: asset folder path '%s'", config.asset_folder_path);
    return TRUE;
}

void resource_system_shutdown()
{
    if (system_state) {
        system_state = 0;
    }
}

b8 resource_system_register_loader(resource_loader new_loader)
{
    if (system_state) {
        u32 count = system_state->config.max_loader_count;
        for (u32 i = 0; i < count; ++i) {
            resource_loader* loader = &system_state->registered_loaders[i];
            if (loader->id != INVALID_ID) {
                if (loader->type == new_loader.type) {
                    LOG_ERROR("resource_system_register_loader: Loader has already been registered");
                    return FALSE;
                }
            }
        }

        for (u32 i = 0; i < count; ++i) {
            if (system_state->registered_loaders[i].id == INVALID_ID) {
                system_state->registered_loaders[i] = new_loader;
                system_state->registered_loaders[i].id = i;
                LOG_TRACE("resource_system_register_loader: Loader with id '%d' registered", system_state->registered_loaders[i].id);
                return TRUE;
            }
        }
    }

    return FALSE;
}

b8 resource_system_load(char const* name, resource_type type, resource* resource)
{
    if (system_state) {
        u32 count = system_state->config.max_loader_count;
        for (u32 i = 0; i < count; ++i) {
            resource_loader* loader = &system_state->registered_loaders[i];
            if (loader->id != INVALID_ID && loader->type == type) {
                return load(name, loader, resource);
            }
        }
    }

    resource->loader_id = INVALID_ID;
    LOG_ERROR("resource_system_load: Loader for required type hasn't yet been registered");
    return FALSE;
}

void resource_system_unload(resource* resource)
{
    if (system_state && resource) {
        if (resource->loader_id != INVALID_ID) {
            resource_loader* loader = &system_state->registered_loaders[resource->loader_id];
            if (loader->id != INVALID_ID && loader->unload) {
                loader->unload(loader, resource);
            }
        }
    }
}

char const* resource_system_asset_folder_path()
{
    if (system_state) {
        return system_state->config.asset_folder_path;
    }

    LOG_WARNING("resource_system_asset_folder_path: Resource system hasn't been initialized yet. Empty string will be returned");
    return "";
}

b8 load(char const* name, resource_loader* loader, resource* resource)
{
    if (!name || !loader || !loader->load || !resource) {
        resource->loader_id = INVALID_ID;
        return FALSE;
    }

    return loader->load(loader, name, resource);
}

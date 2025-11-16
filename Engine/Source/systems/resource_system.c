#include "resource_system.h"

#include "core/logger.h"
#include "resources/loaders/binary_loader.h"
#include "resources/loaders/image_loader.h"
#include "resources/loaders/material_loader.h"
#include "resources/loaders/text_loader.h"

typedef struct Resource_System_State
{
    Resource_System_Config config;
    Resource_Loader* registered_loaders;
} Resource_System_State;

static Resource_System_State* state;

static b8 load(char const* name, Resource_Loader* loader, Resource* resource);

b8 resource_system_startup(u64* const required_memory, void* const block, Resource_System_Config config)
{
    u64 state_required_memory = sizeof(*state);
    u64 loaders_required_memory = config.max_loader_count * sizeof(Resource_Loader);
    *required_memory = state_required_memory + loaders_required_memory;
    if (!block)
    {
        return TRUE;
    }

    state = block;
    state->config = config;
    state->registered_loaders = (char*)state + state_required_memory;
    for (u32 i = 0; i < config.max_loader_count; ++i)
    {
        state->registered_loaders[i].id = INVALID_ID;
    }

    resource_system_register_loader(binary_loader_create());
    resource_system_register_loader(image_loader_create());
    resource_system_register_loader(material_loader_create());
    resource_system_register_loader(text_resource_loader_create());

    return TRUE;
}

void resource_system_shutdown()
{
    for (u32 i = 0; i < state->config.max_loader_count; ++i)
    {
        Resource_Loader* loader = &state->registered_loaders[i];
        if (loader->id != INVALID_ID)
        {
            loader->id = INVALID_ID;
        }
    }

    state = 0;
}

b8 resource_system_register_loader(Resource_Loader new_loader)
{
    for (u32 i = 0; i < state->config.max_loader_count; ++i)
    {
        Resource_Loader* loader = &state->registered_loaders[i];
        if (loader->id != INVALID_ID)
        {
            if (loader->type == new_loader.type)
            {
                LOG_ERROR("resource_system_register_loader: This loader type has already been registered");
                return FALSE;
            }
        }
    }

    for (u32 i = 0; i < state->config.max_loader_count; ++i)
    {
        if (state->registered_loaders[i].id == INVALID_ID)
        {
            state->registered_loaders[i] = new_loader;
            state->registered_loaders[i].id = i;
            return TRUE;
        }
    }
}

b8 resource_system_load(char const* name, resource_type type, Resource* resource)
{
    if (state)
    {
        for (u32 i = 0; i < state->config.max_loader_count; ++i)
        {
            Resource_Loader* loader = &state->registered_loaders[i];
            if (loader->id != INVALID_ID && loader->type == type)
            {
                return load(name, loader, resource);
            }
        }
    }

    LOG_WARNING("resource_system_load: Resource system hasn't been initialized yet");
    resource->loader_id = INVALID_ID;
    LOG_ERROR("resource_system_load: Loader for required type hasn't yet been registered");
    return FALSE;
}

void resource_system_unload(Resource* resource)
{
    if (state && resource)
    {
        if (resource->loader_id != INVALID_ID)
        {
            Resource_Loader* l = &state->registered_loaders[resource->loader_id];
            if (l->id != INVALID_ID && l->unload)
            {
                l->unload(l, resource);
            }
        }
    }

    LOG_WARNING("resource_system_unload: Resource system hasn't been initialized yet");
}

char const* resource_system_asset_folder()
{
    if (state)
    {
        return state->config.asset_folder_path;
    }

    LOG_WARNING("resource_system_asset_folder_path: Resource system hasn't been initialized yet");
    return "";
}

b8 load(char const* name, Resource_Loader* loader, Resource* resource)
{
    if (!name || !loader || !loader->load || !resource)
    {
        resource->loader_id = INVALID_ID;
        return FALSE;
    }

    resource->loader_id = loader->id;
    return loader->load(loader, name, resource);
}

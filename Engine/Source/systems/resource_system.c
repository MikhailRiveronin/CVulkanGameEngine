#include "resource_system.h"

#include "core/logger.h"
#include "systems/memory_system.h"
#include "resources/loaders/binary_loader.h"
#include "resources/loaders/image_loader.h"
#include "resources/loaders/material_loader.h"
#include "resources/loaders/text_loader.h"
#include "resources/loaders/shader_config_loader.h"

typedef struct Resource_System_State
{
    Resource_Loader[RESOURCE_TYPE_ENUM_COUNT] loaders;
} Resource_System_State;

static Resource_System_State* state;

b8 load(char const* name, Resource_Loader* loader, Resource* resource);

b8 resource_system_startup(u64* required_memory, void* block)
{
    u64 state_struct_required_memory = sizeof(*state);
    *required_memory = state_struct_required_memory;

    if (!block)
    {
        return TRUE;
    }

    state = block;

    state->loaders[RESOURCE_TYPE_TEXT] = text_resource_loader_create();
    state->loaders[RESOURCE_TYPE_BINARY] = binary_loader_create();
    state->loaders[RESOURCE_TYPE_IMAGE] = image_loader_create();
    state->loaders[RESOURCE_TYPE_MATERIAL] = material_loader_create();
    state->loaders[RESOURCE_TYPE_SHADER_CONFIG] = shader_config_loader_create();

    return TRUE;
}

void resource_system_shutdown()
{
    if (state)
    {
        memory_system_free(state, sizeof(*state));
        state = 0;
    }
}

b8 resource_system_load(char const* filename, Resource_Type type, Resource* resource)
{
    if (state)
    {
        return load(filename, &state->loaders[type], resource);
    }

    LOG_FATAL("resource_system_load: Loader for required type hasn't yet been registered");
    return FALSE;
}

void resource_system_unload(Resource* resource)
{
    if (state && resource)
    {
        state->loaders[resource->loader_id].unload(resource);
    }

    LOG_WARNING("resource_system_unload: Resource system hasn't been initialized yet");
}

b8 load(char const* filename, Resource_Loader* loader, Resource* resource)
{
    if (!filename || !loader || !loader->load || !resource)
    {
        LOG_FATAL("load: Failed to load resource");
        return FALSE;
    }

    return loader->load(filename, resource);
}

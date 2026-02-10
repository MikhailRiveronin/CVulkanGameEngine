#include "resource_system.h"

#include "core/logger.h"
#include "resources/loaders.h"
#include "systems/memory_system.h"

typedef struct Resource_System_State
{
    Resource_Loader* loaders[RESOURCE_TYPE_ENUM_COUNT];
} Resource_System_State;

static Resource_System_State* state;

bool resource_system_startup()
{
    state = memory_system_allocate(sizeof(*state), MEMORY_TAG_SYSTEMS);
    state->loaders[RESOURCE_TYPE_TEXT] = text_loader_create();
    state->loaders[RESOURCE_TYPE_BINARY] = binary_loader_create();
    state->loaders[RESOURCE_TYPE_IMAGE] = image_loader_create();
    state->loaders[RESOURCE_TYPE_MATERIAL] = material_loader_create();
    state->loaders[RESOURCE_TYPE_SHADER_CONFIG] = shader_config_loader_create();
    return true;
}

void resource_system_shutdown()
{
    if (state)
    {
        for (u32 i = 0; i < RESOURCE_TYPE_ENUM_COUNT; ++i)
        {
            memory_system_free(&state->loaders[i], MEMORY_TAG_LOADERS);
        }

        memory_system_free(state, sizeof(*state), MEMORY_TAG_SYSTEMS);
        state = 0;
    }

    LOG_WARNING("resource_system_shutdown: Resource system hasn't been started up yet");
}

bool resource_system_load(Resource_Type type, char const* filename, Resource* resource)
{
    if (!filename || !resource)
    {
        LOG_FATAL("resource_system_load: Invalid parameters");
        return false;
    }

    if (state)
    {
        resource->type = type;
        return state->loaders[resource->type]->load(filename, resource);
    }

    LOG_FATAL("resource_system_load: Resource system hasn't been started up yet");
    return false;
}

void resource_system_unload(Resource* resource)
{
    if (!resource)
    {
        LOG_FATAL("resource_system_unload: Invalid parameters");
        return;
    }

    if (state)
    {
        state->loaders[resource->type]->unload(resource);
    }

    LOG_WARNING("resource_system_load: Resource system hasn't been started up yet");
}

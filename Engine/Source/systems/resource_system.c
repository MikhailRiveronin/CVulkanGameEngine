#include "resource_system.h"

#include "core/logger.h"
#include "core/string_utils.h"

// Known resource loaders.
#include "resources/loaders/text_loader.h"
#include "resources/loaders/binary_loader.h"
#include "resources/loaders/image_loader.h"
#include "resources/loaders/material_loader.h"

typedef struct resource_system_state {
    resource_system_config config;
    resource_loader* registered_loaders;
} resource_system_state;

static resource_system_state* system_state;

b8 load(const char* name, resource_loader* loader, resource* out_resource);

b8 resource_system_startup(u64* memory_requirement, void* state, resource_system_config config) {
    if (config.max_loader_count == 0) {
        LOG_FATAL("resource_system_startup failed because config.max_loader_count==0.");
        return FALSE;
    }

    *memory_requirement = sizeof(resource_system_state) + (sizeof(resource_loader) * config.max_loader_count);

    if (!state) {
        return TRUE;
    }

    system_state = state;
    system_state->config = config;

    void* array_block = (char*)state + sizeof(resource_system_state);
    system_state->registered_loaders = array_block;

    // Invalidate all loaders
    u32 count = config.max_loader_count;
    for (u32 i = 0; i < count; ++i) {
        system_state->registered_loaders[i].id = INVALID_ID;
    }

    // NOTE: Auto-register known loader types here.
    resource_system_register_loader(text_resource_loader_create());
    resource_system_register_loader(binary_resource_loader_create());
    resource_system_register_loader(image_resource_loader_create());
    resource_system_register_loader(material_resource_loader_create());

    LOG_INFO("Resource system initialized with base path '%s'.", config.asset_base_path);

    return TRUE;
}

void resource_system_shutdown(void* state)
{
    if (system_state) {
        system_state = 0;
    }
}

b8 resource_system_register_loader(resource_loader loader)
{
    if (system_state) {
        u32 count = system_state->config.max_loader_count;
        // Ensure no loaders for the given type already exist
        for (u32 i = 0; i < count; ++i) {
            resource_loader* l = &system_state->registered_loaders[i];
            if (l->id != INVALID_ID) {
                if (l->type == loader.type) {
                    LOG_ERROR("resource_system_register_loader - Loader of type %d already exists and will not be registered.", loader.type);
                    return FALSE;
                } else if (loader.custom_type && string_length(loader.custom_type) > 0 && string_equali(l->custom_type, loader.custom_type)) {
                    LOG_ERROR("resource_system_register_loader - Loader of custom type %s already exists and will not be registered.", loader.custom_type);
                    return FALSE;
                }
            }
        }
        for (u32 i = 0; i < count; ++i) {
            if (system_state->registered_loaders[i].id == INVALID_ID) {
                system_state->registered_loaders[i] = loader;
                system_state->registered_loaders[i].id = i;
                LOG_TRACE("Loader registered.");
                return TRUE;
            }
        }
    }

    return FALSE;
}

b8 resource_system_load(const char* name, resource_type type, resource* out_resource)
{
    if (system_state && type != RESOURCE_TYPE_CUSTOM) {
        // Select loader.
        u32 count = system_state->config.max_loader_count;
        for (u32 i = 0; i < count; ++i) {
            resource_loader* l = &system_state->registered_loaders[i];
            if (l->id != INVALID_ID && l->type == type) {
                return load(name, l, out_resource);
            }
        }
    }

    out_resource->loader_id = INVALID_ID;
    LOG_ERROR("resource_system_load - No loader for type %d was found.", type);
    return FALSE;
}

b8 resource_system_load_custom(const char* name, const char* custom_type, resource* out_resource)
{
    if (system_state && custom_type && string_length(custom_type) > 0) {
        // Select loader.
        u32 count = system_state->config.max_loader_count;
        for (u32 i = 0; i < count; ++i) {
            resource_loader* l = &system_state->registered_loaders[i];
            if (l->id != INVALID_ID && l->type == RESOURCE_TYPE_CUSTOM && string_equali(l->custom_type, custom_type)) {
                return load(name, l, out_resource);
            }
        }
    }

    out_resource->loader_id = INVALID_ID;
    LOG_ERROR("resource_system_load_custom - No loader for type %s was found.", custom_type);
    return FALSE;
}

void resource_system_unload(resource* resource) {
    if (system_state && resource) {
        if (resource->loader_id != INVALID_ID) {
            resource_loader* l = &system_state->registered_loaders[resource->loader_id];
            if (l->id != INVALID_ID && l->unload) {
                l->unload(l, resource);
            }
        }
    }
}

const char* resource_system_base_path() {
    if (system_state) {
        return system_state->config.asset_base_path;
    }

    LOG_ERROR("resource_system_base_path called before initialization, returning empty string.");
    return "";
}

b8 load(const char* name, resource_loader* loader, resource* out_resource) {
    if (!name || !loader || !loader->load || !out_resource) {
        out_resource->loader_id = INVALID_ID;
        return FALSE;
    }

    out_resource->loader_id = loader->id;
    return loader->load(loader, name, out_resource);
}

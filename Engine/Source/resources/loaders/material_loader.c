#include "material_loader.h"

#include "core/logger.h"
#include "core/string_utils.h"
#include "systems/memory_system.h"
#include "third_party/cJSON/cJSON.h"

#include "config.h"


#include "platform/filesystem.h"
#include "resources/resources.h"
#include "systems/resource_system.h"
#include "third_party/cglm/cglm.h"

b8 load_material(Resource_Loader* loader, char const* name, Resource* resource);
void unload_material(Resource_Loader* loader, Resource* resource);

Resource_Loader material_loader_create()
{
    Resource_Loader loader;
    loader.type = RESOURCE_TYPE_MATERIAL;
    loader.load = load_material;
    loader.unload = unload_material;
    return loader;
}

b8 load_material(char const* filename, Resource* resource)
{
    char path[256];
    string_format(path, "%s/%s/%s", ASSETS_DIR, "materials", filename);

    cJSON* json = cJSON_Parse(path);
    if (!json)
    {
        char const* error = cJSON_GetErrorPtr();
        if (error)
        {
            LOG_FATAL("load_material: %s", error);
            return FALSE;
        }

        LOG_FATAL("load_material: Unknown parsing error");
        return FALSE;
    }

    Material_Config* config = memory_system_allocate(sizeof(*config), MEMORY_TAG_MATERIAL_INSTANCE);
    cJSON* version = cJSON_GetObjectItemCaseSensitive(json, "version");
    if (cJSON_IsString(version) && version->valuestring)
    {
        string_copy(config->version, version->valuestring);
    }
    else
    {
        LOG_FATAL("load_material: Failed to parse version");
        return FALSE;
    }

    cJSON* name = cJSON_GetObjectItemCaseSensitive(json, "name");
    if (cJSON_IsString(name) && name->valuestring)
    {
        string_copy(config->name, name->valuestring);
    }
    else
    {
        LOG_FATAL("load_material: Failed to parse name");
        return FALSE;
    }

    u8 index = 0;
    cJSON* elem = 0;
    cJSON* diffuse_color = cJSON_GetObjectItemCaseSensitive(json, "diffuse_color");
    cJSON_ArrayForEach(elem, diffuse_color)
    {
        if (cJSON_IsNumber(elem))
        {
            config->diffuse_colour.raw[index] = (float)elem->valuedouble;
            index++;
        }
        else
        {
            LOG_FATAL("load_material: Failed to parse duffuse_color");
            return FALSE;
        }
    }

    cJSON* diffuse_map_name = cJSON_GetObjectItemCaseSensitive(json, "diffuse_map_name");
    if (cJSON_IsString(diffuse_map_name) && diffuse_map_name->valuestring)
    {
        string_copy(config->diffuse_texture_name, diffuse_map_name->valuestring);
    }
    else
    {
        LOG_FATAL("load_material: Failed to parse duffuse_map_name");
        return FALSE;
    }

    cJSON* type = cJSON_GetObjectItemCaseSensitive(json, "type");
    if (cJSON_IsString(type) && type->valuestring)
    {
        if (string_equal(type->valuestring, "world"))
        {
            config->type = MATERIAL_TYPE_WORLD;
        }
        else if (string_equal(type->valuestring, "ui"))
        {
            config->type = MATERIAL_TYPE_UI;
        }
        else
        {
            LOG_FATAL("load_material: Unknown material type");
            return FALSE;
        }
    }
    else
    {
        LOG_FATAL("load_material: Failed to parse type");
        return FALSE;
    }

    cJSON* auto_release = cJSON_GetObjectItemCaseSensitive(json, "auto_release");
    if (cJSON_IsBool(auto_release))
    {
        if (cJSON_IsTrue(auto_release))
        {
            config->auto_release = TRUE;
        }
        else
        {
            config->auto_release = FALSE;
        }
    }
    else
    {
        LOG_FATAL("load_material: Failed to parse auto_release");
        return FALSE;
    }

    cJSON_Delete(json);

    resource->name = filename;
    resource->data = config;
    resource->data_size = sizeof(*config);

    return TRUE;
}

void unload_material(Resource* resource)
{
    memory_system_free(resource->data, resource->data_size, MEMORY_TAG_MATERIAL_INSTANCE);
    resource->loader_id = INVALID_ID;
    resource->data = 0;
    resource->data_size = 0;
}

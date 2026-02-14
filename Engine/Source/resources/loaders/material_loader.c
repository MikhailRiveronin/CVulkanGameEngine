#include "material_loader.h"

#include "config.h"
#include "core/logger.h"
#include "core/string_utils.h"
#include "systems/memory_system.h"
#include "third_party/cJSON/cJSON.h"

static bool load(char const* filename, Resource* resource);
static void unload(Resource* resource);

Resource_Loader* material_loader_create()
{
    Resource_Loader* loader = memory_system_allocate(sizeof(*loader), MEMORY_TAG_LOADERS);
    loader->load = load;
    loader->unload = unload;
    return loader;
}

bool load(char const* filename, Resource* resource)
{
    if (!filename || !resource)
    {
        LOG_WARNING("material loader unload: Invalid parameters");
    }

    char path[256];
    string_format(path, "%s/%s/%s", ASSETS_DIR, "materials", filename);
    cJSON* json = cJSON_Parse(path);
    if (!json)
    {
        char const* error = cJSON_GetErrorPtr();
        if (error)
        {
            LOG_FATAL("material_loader load: %s", error);
            return false;
        }

        LOG_FATAL("material_loader load: Unknown parsing error");
        return false;
    }

    Material_Config_Resource* config = memory_system_allocate(sizeof(*config), MEMORY_TAG_RESOURCES);
    cJSON* version = cJSON_GetObjectItemCaseSensitive(json, "version");
    if (cJSON_IsString(version) && version->valuestring)
    {
        string_copy(config->version, version->valuestring);
    }
    else
    {
        LOG_FATAL("material_loader load: Failed to parse version");
        return false;
    }

    cJSON* name = cJSON_GetObjectItemCaseSensitive(json, "name");
    if (cJSON_IsString(name) && name->valuestring)
    {
        string_copy(config->name, name->valuestring);
    }
    else
    {
        LOG_FATAL("load: Failed to parse name");
        return false;
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
            LOG_FATAL("load: Failed to parse duffuse_color");
            return false;
        }
    }

    cJSON* diffuse_map_name = cJSON_GetObjectItemCaseSensitive(json, "diffuse_map_name");
    if (cJSON_IsString(diffuse_map_name) && diffuse_map_name->valuestring)
    {
        string_copy(config->diffuse_texture_name, diffuse_map_name->valuestring);
    }
    else
    {
        LOG_FATAL("load: Failed to parse duffuse_map_name");
        return false;
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
            LOG_FATAL("load: Unknown material type");
            return false;
        }
    }
    else
    {
        LOG_FATAL("load: Failed to parse type");
        return false;
    }

    cJSON* auto_release = cJSON_GetObjectItemCaseSensitive(json, "auto_release");
    if (cJSON_IsBool(auto_release))
    {
        if (cJSON_IsTrue(auto_release))
        {
            config->auto_release = true;
        }
        else
        {
            config->auto_release = false;
        }
    }
    else
    {
        LOG_FATAL("load: Failed to parse auto_release");
        return false;
    }

    cJSON_Delete(json);
    resource->data = config;
    resource->size = sizeof(*config);
    return true;
}

void unload(Resource* resource)
{
    if (!resource)
    {
        LOG_WARNING("material loader unload: Invalid parameters");
    }

    memory_system_free(resource->data, resource->size, MEMORY_TAG_RESOURCES);
    resource->data = 0;
    resource->size = 0;
}

#include "shader_config_loader.h"

#include "config.h"
#include "core/logger.h"
#include "core/string_utils.h"
#include "systems/memory_system.h"
#include "third_party/cJSON/cJSON.h"

static bool load(char const* filename, Resource* resource);
static void unload(Resource* resource);

Resource_Loader* shader_config_loader_create()
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
        LOG_FATAL("shader_config_loader load: Invalid parameters");
        return false;
    }

    char path[256];
    string_format(path, "%s/%s/%s", ASSETS_DIR, "shaders", filename);
    cJSON* json = cJSON_Parse(path);
    if (!json)
    {
        char const* error = cJSON_GetErrorPtr();
        if (error)
        {
            LOG_FATAL("shader_config_loader load: %s", error);
            return FALSE;
        }

        LOG_FATAL("shader_config_loader load: Unknown parsing error");
        return FALSE;
    }

    Shader_Config_Resource* config = memory_system_allocate(sizeof(*config), MEMORY_TAG_RESOURCES);

    cJSON* name = cJSON_GetObjectItemCaseSensitive(json, "name");
    if (cJSON_IsString(name) && name->valuestring)
    {
        strcpy(config->name, name->valuestring);
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse name");
        return FALSE;
    }

    cJSON* renderpass = cJSON_GetObjectItemCaseSensitive(json, "renderpass");
    if (cJSON_IsString(renderpass) && renderpass->valuestring)
    {
        strcpy(config->renderpass_name, renderpass->valuestring);
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse renderpass");
        return FALSE;
    }

    config->stages = DYNAMIC_ARRAY_CREATE(char const*);
    cJSON* elem = 0;
    cJSON* stages = cJSON_GetObjectItemCaseSensitive(json, "stages");
    if (cJSON_IsArray(stages))
    {
        cJSON_ArrayForEach(elem, stages)
        {
            if (cJSON_IsString(elem) && elem->valuestring)
            {
                dynamic_array_push(config->stages, elem->valuestring);
            }
            else
            {
                LOG_FATAL("shader_config_loader load: Failed to parse stages");
                return FALSE;
            }
        }
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse stages");
        return FALSE;
    }

    config->spv_binaries = DYNAMIC_ARRAY_CREATE(char const*);
    cJSON* spv_binaries = cJSON_GetObjectItemCaseSensitive(json, "spv_binaries");
    if (cJSON_IsArray(spv_binaries))
    {
        cJSON_ArrayForEach(elem, spv_binaries)
        {
            if (cJSON_IsString(elem) && elem->valuestring)
            {
                DYNAMIC_ARRAY_PUSH(&config->spv_binaries, elem->valuestring);
            }
            else
            {
                LOG_FATAL("shader_config_loader load: Failed to parse spv binaries");
                return FALSE;
            }
        }
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse spv binaries");
        return FALSE;
    }

    ASSERT(config->stages->size == config->spv_binaries->size);

    config->attributes = DYNAMIC_ARRAY_CREATE(Vertex_Attribute_Config);
    cJSON* attributes = cJSON_GetObjectItemCaseSensitive(json, "attributes");
    if (cJSON_IsObject(attributes))
    {
        cJSON_ArrayForEach(elem, attributes)
        {
            Vertex_Attribute_Config attribute;
            strcpy(attribute.name, elem->string);

            cJSON* item = cJSON_GetObjectItemCaseSensitive(elem, elem->string);
            if (cJSON_IsString(item) && item->valuestring)
            {
                if (string_equal(item->valuestring, "vec2"))
                {
                    attribute.size = 8;
                    attribute.type = VK_FORMAT_R32G32_SFLOAT;
                }
                else if (string_equal(item->valuestring, "vec3"))
                {
                    attribute.size = 12;
                    attribute.type = VK_FORMAT_R32G32B32_SFLOAT;
                }
                else
                {
                    LOG_FATAL("shader_config_loader load: Failed to parse attributes");
                    return FALSE;
                }
            }
            else
            {
                LOG_FATAL("shader_config_loader load: Failed to parse attributes");
                return FALSE;
            }

            DYNAMIC_ARRAY_PUSH(&config->attributes, attribute);
        }
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse attributes");
        return FALSE;
    }

    cJSON* per_material = cJSON_GetObjectItemCaseSensitive(json, "per-material");
    if (cJSON_IsBool(per_material))
    {
        if (cJSON_IsTrue(per_material))
        {
            config->per_material = TRUE;
        }
        else
        {
            config->per_material = FALSE;
        }
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse per-material");
        return FALSE;
    }

    cJSON* per_object = cJSON_GetObjectItemCaseSensitive(json, "per-object");
    if (cJSON_IsBool(per_object))
    {
        if (cJSON_IsTrue(per_object))
        {
            config->per_object = TRUE;
        }
        else
        {
            config->per_object = FALSE;
        }
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse per-object");
        return FALSE;
    }

    config->uniforms = DYNAMIC_ARRAY_CREATE(Uniform_Config);
    cJSON* uniforms = cJSON_GetObjectItemCaseSensitive(json, "uniforms");
    if (cJSON_IsArray(uniforms))
    {
        u8 i = 0;
        cJSON_ArrayForEach(elem, uniforms)
        {
            if (cJSON_IsObject(elem))
            {
                cJSON* item = 0;
                cJSON_ArrayForEach(item, elem)
                {
                    Uniform_Config uniform;
                    strcpy(uniform.name, item->string);

                    cJSON* item = cJSON_GetObjectItemCaseSensitive(elem, elem->string);
                    if (cJSON_IsString(item) && item->valuestring)
                    {
                        if (string_equal(item->valuestring, "vec4"))
                        {
                            uniform.size = 16;
                            uniform.type = UNIFORM_TYPE_VEC4;
                        }
                        else if (string_equal(item->valuestring, "mat4"))
                        {
                            uniform.size = 64;
                            uniform.type = UNIFORM_TYPE_MAT4;
                        }
                        else if (string_equal(item->valuestring, "sampler"))
                        {
                            attribute.size = 0;
                            attribute.type = UNIFORM_TYPE_SAMPLER;
                        }
                        else
                        {
                            LOG_FATAL("shader_config_loader load: Failed to parse uniforms");
                            return FALSE;
                        }

                        uniform.scope = DESCRIPTOR_SET_SCOPE_PER_FRAME + i;
                        DYNAMIC_ARRAY_PUSH(&config->uniforms, uniform);
                    }
                    else
                    {
                        LOG_FATAL("shader_config_loader load: Failed to parse uniforms");
                        return FALSE;
                    }
                }

                i++;
            }
            else
            {
                LOG_FATAL("shader_config_loader load: Failed to parse uniforms");
                return FALSE;
            }
        }
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse uniforms");
        return FALSE;
    }

    cJSON_Delete(json);

    resource->data_size = sizeof(*config);
    resource->data = config;
}

void unload(Resource* resource)
{
    if (!resource)
    {
        LOG_WARNING("shader_config_loader unload: Invalid parameters");
    }

    dynamic_array_destroy(((Shader_Config_Resource*)resource->data)->attributes);
    dynamic_array_destroy(((Shader_Config_Resource*)resource->data)->attributes);
    dynamic_array_destroy(((Shader_Config_Resource*)resource->data)->spv_binaries);
    dynamic_array_destroy(((Shader_Config_Resource*)resource->data)->stages);
    memory_system_free(resource->data, resource->data_size, MEMORY_TAG_RESOURCES);
}

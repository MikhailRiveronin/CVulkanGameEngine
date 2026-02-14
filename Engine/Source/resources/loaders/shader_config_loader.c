#include "shader_config_loader.h"

#include "config.h"
#include "core/logger.h"
#include "core/string_utils.h"
#include "renderer/vulkan_structure_initializers.h"
#include "systems/memory_system.h"
#include "third_party/cJSON/cJSON.h"
#include "third_party/cglm/struct.h"

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
            return false;
        }

        LOG_FATAL("shader_config_loader load: Unknown parsing error");
        return false;
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
        return false;
    }

    cJSON* renderpass = cJSON_GetObjectItemCaseSensitive(json, "renderpass");
    if (cJSON_IsString(renderpass) && renderpass->valuestring)
    {
        strcpy(config->renderpass_name, renderpass->valuestring);
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse renderpass");
        return false;
    }

    config->stages = DYNAMIC_ARRAY_CREATE(VkShaderStageFlagBits);
    cJSON* elem = 0;
    cJSON* stages = cJSON_GetObjectItemCaseSensitive(json, "stages");
    if (cJSON_IsArray(stages))
    {
        cJSON_ArrayForEach(elem, stages)
        {
            if (cJSON_IsString(elem) && elem->valuestring)
            {
                if (string_equal(elem->valuestring, "vertex"))
                {
                    dynamic_array_push_back(config->stages, VK_SHADER_STAGE_VERTEX_BIT);
                }
                else if (string_equal(elem->valuestring, "fragment"))
                {
                    dynamic_array_push_back(config->stages, VK_SHADER_STAGE_FRAGMENT_BIT);
                }
                else
                {
                    LOG_FATAL("shader_config_loader load: Unsupported shader stage");
                    return false;
                }
            }
            else
            {
                LOG_FATAL("shader_config_loader load: Failed to parse stages");
                return false;
            }
        }
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse stages");
        return false;
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
                return false;
            }
        }
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse spv binaries");
        return false;
    }

    ASSERT(config->stages->size == config->spv_binaries->size);

    Dynamic_Array* vertex_binding_descriptions = DYNAMIC_ARRAY_CREATE(VkVertexInputBindingDescription);
    Dynamic_Array* vertex_attribute_descriptions = DYNAMIC_ARRAY_CREATE(VkVertexInputAttributeDescription);
    cJSON* attributes = cJSON_GetObjectItemCaseSensitive(json, "attributes");
    if (cJSON_IsObject(attributes))
    {
        u32 i = 0;
        cJSON_ArrayForEach(elem, attributes)
        {
            cJSON* item = cJSON_GetObjectItemCaseSensitive(elem, elem->string);
            if (cJSON_IsString(item) && item->valuestring)
            {
                VkVertexInputBindingDescription binding;
                VkVertexInputAttributeDescription attribute;
                if (string_equal(item->valuestring, "vec2"))
                {
                    binding = vertex_input_binding_description(i, sizeof(vec2s));
                    attribute = vertex_input_attribute_description(i, binding.binding, VK_FORMAT_R32G32_SFLOAT, 0);
                }
                else if (string_equal(item->valuestring, "vec3"))
                {
                    binding = vertex_input_binding_description(i, sizeof(vec3s));
                    attribute = vertex_input_attribute_description(i, binding.binding, VK_FORMAT_R32G32B32_SFLOAT, 0);
                }
                else
                {
                    LOG_FATAL("shader_config_loader load: Failed to parse attributes");
                    return false;
                }

                dynamic_array_push_back(vertex_binding_descriptions, &binding);
                dynamic_array_push_back(vertex_attribute_descriptions, &attribute);
            }
            else
            {
                LOG_FATAL("shader_config_loader load: Failed to parse attributes");
                return false;
            }
        }

        dynamic_array_destroy(vertex_binding_descriptions);
        dynamic_array_destroy(vertex_attribute_descriptions);
        config->vertex_input_state = pipeline_vertex_input_state_create_info(vertex_binding_descriptions->data, vertex_binding_descriptions->size, vertex_attribute_descriptions->data, vertex_attribute_descriptions->size);
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse attributes");
        return false;
    }

    cJSON* per_material = cJSON_GetObjectItemCaseSensitive(json, "per-material");
    if (cJSON_IsBool(per_material))
    {
        if (cJSON_IsTrue(per_material))
        {
            config->per_material = true;
        }
        else
        {
            config->per_material = false;
        }
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse per-material");
        return false;
    }

    cJSON* per_object = cJSON_GetObjectItemCaseSensitive(json, "per-object");
    if (cJSON_IsBool(per_object))
    {
        if (cJSON_IsTrue(per_object))
        {
            config->per_object = true;
        }
        else
        {
            config->per_object = false;
        }
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse per-object");
        return false;
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
                            uniform.size = 0;
                            uniform.type = UNIFORM_TYPE_SAMPLER;
                        }
                        else
                        {
                            LOG_FATAL("shader_config_loader load: Failed to parse uniforms");
                            return false;
                        }

                        uniform.scope = DESCRIPTOR_SET_SCOPE_PER_FRAME + i;
                        DYNAMIC_ARRAY_PUSH(&config->uniforms, uniform);
                    }
                    else
                    {
                        LOG_FATAL("shader_config_loader load: Failed to parse uniforms");
                        return false;
                    }
                }

                i++;
            }
            else
            {
                LOG_FATAL("shader_config_loader load: Failed to parse uniforms");
                return false;
            }
        }
    }
    else
    {
        LOG_FATAL("shader_config_loader load: Failed to parse uniforms");
        return false;
    }

    cJSON_Delete(json);

    resource->size = sizeof(*config);
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
    memory_system_free(resource->data, resource->size, MEMORY_TAG_RESOURCES);
}

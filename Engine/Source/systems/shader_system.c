#include "shader_system.h"

#include "containers/string_map.h"
#include "core/string_utils.h"
#include "memory_system.h"
#include "renderer/renderer.h"

typedef struct Shader_System_State
{
    Shader_System_Config config;
    String_Map shader_ids;
    Shader* shaders;
    u32 current_shader_id;
} Shader_System_State;

static Shader_System_State* state;

static void destroy_shader(Shader* shader);

bool shader_system_startup(Shader_System_Config const* config)
{
    state = memory_system_allocate(sizeof(*state), MEMORY_TAG_SYSTEMS);
    state->config = *config;
    state->shader_ids = string_map_create("u32", config->max_shader_count, sizeof(((Shader*)0)->id));
    state->shaders = memory_system_allocate(config->max_shader_count * sizeof(*state->shaders), MEMORY_TAG_SYSTEMS);
    state->current_shader_id = INVALID_ID;

    // TODO: Check if really needed
    for (u32 i = 0; i < state->config.max_shader_count; ++i)
    {
        state->shaders[i].config = memory_system_allocate(sizeof(*state->shaders[i].config), MEMORY_TAG_SYSTEMS);
        state->shaders[i].id = INVALID_ID;
    }

    return true;
}

void shader_system_shutdown()
{
    if (state)
    {
        string_map_destroy(state->shader_ids);
        for (u32 i = 0; i < state->config.max_shader_count; ++i)
        {
            memory_system_free(state->shaders[i].config, sizeof(*state->shaders[i].config), MEMORY_TAG_SYSTEMS);
            if (state->shaders[i].id != INVALID_ID)
            {
                renderer_shader_destroy(&state->shaders[i]);
            }
        }

        memory_system_free(state->shaders, config->max_shader_count * sizeof(*state->shaders), MEMORY_TAG_SYSTEMS);
        memory_system_free(state, sizeof(*state), MEMORY_TAG_SYSTEMS);
        state = 0;
    }
}

bool shader_system_create(Shader_Config_Resource const* config)
{
    u32 id = INVALID_ID;
    for (u32 i = 0; i < state->config.max_shader_count; ++i)
    {
        if (state->shaders[i].id == INVALID_ID)
        {
            id = i;
        }
    }

    if (id == INVALID_ID)
    {
        LOG_FATAL("shader_system_create: Failed to find free slot to create new shader");
        return false;
    }

    Shader* shader = &state->shaders[id];
    shader->id = id;
    // strncpy(shader->name, config->name, sizeof(shader->name) - 1);
    // shader->state = SHADER_STATE_UNINITIALIZED;
    // shader->use_instances = config->use_instances;
    // shader->use_locals = config->use_local;
    // shader->push_constant_range_count = 0;
    // shader->bound_instance_id = INVALID_ID;
    // shader->attribute_stride = 0;
    // memory_system_zero(shader->push_constant_ranges, 32 * sizeof(Memory_Range));

    shader->attributes = DYNAMIC_ARRAY_CREATE(Vertex_Attribute);
    shader->global_textures = DYNAMIC_ARRAY_CREATE(Texture*);
    shader->uniforms = DYNAMIC_ARRAY_CREATE(Uniform_Buffer);

    // Arbitrarily chosen size
    shader->uniform_indices = STRING_MAP_CREATE(u16, 1024);




    // DARRAY_DEFINE(Texture*, indices, indexCount, MEMORY_TAG_RENDERER);

    if (HASHTABLE_CREATE(u16, 1024, INVALID_ID, &shader->uniform_buffer_index_lut))
    {
        LOG_FATAL("shader_system_create: Failed to create hash table");
        return false;
    }




    return true;
}

void destroy_shader(Shader* shader)
{

}

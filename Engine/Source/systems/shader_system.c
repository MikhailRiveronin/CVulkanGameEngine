#include "shader_system.h"

#include "containers/darray.h"
#include "core/string_utils.h"
#include "memory_system.h"

typedef struct Shader_System_State
{
    Shader_System_Config config;
    Hashtable lut; // shader name -> shader id
    void* lut_block;
    Shader* shaders;
    u32 current_shader_id;
} Shader_System_State;

Shader_System_State* state;

void destroy_shader(Shader* shader);

b8 shader_system_startup(u64* required_memory, void const* block, Shader_System_Config const* config)
{
    if (config->max_shader_count == 0)
    {
        LOG_FATAL("shader_system_startup: Invalid input parameters");
        return FALSE;
    }

    u64 state_struct_required_memory = sizeof(*state);
    u64 lut_reqired_memory = config->max_shader_count * sizeof(((Shader*)0)->id);
    u64 shader_array_reqired_memory = config->max_shader_count * sizeof(*state->shaders);
    *required_memory = state_struct_required_memory + lut_reqired_memory + shader_array_reqired_memory;

    if (!block)
    {
        return TRUE;
    }

    state = block;
    state->config = *config;
    state->lut_block = (char*)state + state_struct_required_memory;
    state->shaders = (char*)state->lut_block + lut_reqired_memory;
    state->current_shader_id = INVALID_ID;

    if (!hashtable_create(sizeof(((Shader*)0)->id), state->config.max_shader_count, state->lut_block, FALSE, &state->lut))
    {
        LOG_FATAL("shader_system_startup: Failed to fiil the hash table");
        return FALSE;
    }

    u32 invalid_id = INVALID_ID;
    if (!hashtable_fill(&state->lut, &invalid_id))
    {
        LOG_FATAL("shader_system_startup: Failed to fiil the hash table");
        return FALSE;
    }

    for (u32 i = 0; i < state->config.max_shader_count; ++i)
    {
        state->shaders[i].id = INVALID_ID;
    }

    return TRUE;
}

void shader_system_shutdown()
{
    if (state)
    {
        hashtable_destroy(&state->lut);

        for (u32 i = 0; i < state->config.max_shader_count; ++i)
        {
            Shader* shader = &state->shaders[i];
            if (shader->id != INVALID_ID)
            {
                shader_destroy(shader);
            }
        }

        memory_system_zero(state, sizeof(*state));
        state = 0;
    }
}

b8 shader_system_create(Shader_Config const* config)
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
        return FALSE;
    }

    Shader* shader = &state->shaders[id];
    shader->id = id;
    shader->name = string_duplicate(config->name);
    shader->state = SHADER_STATE_NULL;
    shader->use_instances = config->use_instances;
    shader->use_locals = config->use_local;
    shader->push_constant_range_count = 0;
    shader->bound_instance_id = INVALID_ID;
    shader->attribute_stride = 0;
    memory_system_zero(shader->push_constant_ranges, 32 * sizeof(Memory_Range));

    DYNAMIC_ARRAY_CREATE(Texture*, shader->global_textures);
    // DARRAY_DEFINE(Texture*, indices, indexCount, MEMORY_TAG_RENDERER);

    if (HASHTABLE_CREATE(u16, 1024, INVALID_ID, &shader->uniform_buffer_index_lut))
    {
        LOG_FATAL("shader_system_create: Failed to create hash table");
        return FALSE;
    }




    return TRUE;
}

void destroy_shader(Shader* shader)
{

}

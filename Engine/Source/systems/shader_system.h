#pragma once

#include "defines.h"
#include "containers/dynamic_array.h"
#include "containers/hashtable.h"
#include "resources/resource_types.h"

/**
/* @brief Shader system configuration.
 */
typedef struct Shader_System_Config
{
    u8 max_shader_count;
    u8 max_uniform_buffer_count;
    u8 max_global_texture_count;
    u8 max_instance_texture_count;
} Shader_System_Config;

/**
 * @brief The current state of a shader.
 */
typedef enum Shader_State
{
    SHADER_STATE_NULL,
    SHADER_STATE_UNINITIALIZED,
    SHADER_STATE_INITIALIZED,
} Shader_State;

/**
 * @brief An entry in the uniform array.
 */
typedef struct Uniform_Buffer
{
    u64 offset;
    u16 location;
    u16 index;
    u16 size;
    u8 set_index;
    Shader_Scope scope;
    VkFormat type;
} Uniform_Buffer;

/**
 * @brief A shader vertex attribute.
 */
typedef struct Vertex_Attribute
{
    char* name;
    VkFormat type;
    u32 size;
} Vertex_Attribute;

/**
 * @brief A shader on the frontend.
 */
typedef struct Shader
{
    u32 id;
    char* name;

    b8 use_instances;
    b8 use_locals;

    u64 required_ubo_alignment;

    u64 global_ubo_size;
    u64 global_ubo_stride;
    u64 global_ubo_offset;

    u64 instance_ubo_size;
    u64 instance_ubo_stride;

    u64 push_constant_size;
    u64 push_constant_stride;

    Dynamic_Array global_textures;
    u8 instance_texture_count;

    Shader_Scope bound_scope;

    u32 bound_instance_id;
    u32 bound_ubo_offset;

    Hashtable uniform_buffer_index_lut;

    Uniform_Buffer* uniform_buffers;
    Vertex_Attribute* attributes;

    Shader_State state;

    u8 push_constant_range_count;
    Memory_Range push_constant_ranges[32];
    u16 attribute_stride;

    void* internal_data;
} Shader;

b8 shader_system_startup(u64* required_memory, void const* block, Shader_System_Config const* config);
void shader_system_shutdown();

b8 shader_system_create(Shader_Config const* config);


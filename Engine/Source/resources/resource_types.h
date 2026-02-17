#pragma once

#include "defines.h"
#include "containers/dynamic_array.h"

// #include "third_party/cglm/struct.h"










typedef struct Shader_Config_Resource
{
    char name[32];
    char renderpass_name[32];

    Dynamic_Array* stages;
    Dynamic_Array* spv_binaries;

    VkPipelineVertexInputStateCreateInfo vertex_input_state;




    bool per_material;
    bool per_object;

    // Dynamic_Array* attributes;
    Dynamic_Array* uniforms;

    u32 max_descriptor_set_count;
} Shader_Config_Resource;






typedef struct Image_Resource
{
    u8* pixels;
    u32 width;
    u32 height;
    u8 channel_count;
} Image_Resource;

#define TEXTURE_NAME_MAX_LENGTH 128



typedef enum Texture_Use
{
    TEXTURE_USE_UNKNOWN = 0x00,
    TEXTURE_USE_MAP_DIFFUSE = 0x01
} Texture_Use;

typedef struct Texture_Map
{
    Texture* texture;
    Texture_Use use;
} Texture_Map;

#define MAX_MATERIAL_NAME_LENGTH 128

typedef enum Material_Type
{
    MATERIAL_TYPE_WORLD,
    MATERIAL_TYPE_UI
} Material_Type;

// typedef struct Material_Config_Resource
// {
//     char version[4];
//     char name[MAX_MATERIAL_NAME_LENGTH];
//     vec4s diffuse_colour;
//     char diffuse_texture_name[TEXTURE_NAME_MAX_LENGTH];
//     Material_Type type;
//     bool auto_release;
// } Material_Config_Resource;

typedef struct Material
{
    char name[MAX_MATERIAL_NAME_LENGTH];
    u32 id;
    u32 backend_id;
    u32 generation;
    Material_Type type;
    vec4s diffuse_color;
    Texture_Map diffuse_map;
} Material;

#define GEOMETRY_MAX_NAME_LENGTH 256

typedef struct Geometry
{
    u32 id;
    u32 internal_id;
    char name[GEOMETRY_MAX_NAME_LENGTH];
    u32 generation;
    Material* material;
} Geometry;











// typedef struct Vertex_Attribute_Config
// {
//     char name[32];
//     u8 size;
//     VkFormat type;
// } Vertex_Attribute_Config;

typedef enum Uniform_Type
{
    UNIFORM_TYPE_VEC4,
    UNIFORM_TYPE_MAT4,
    UNIFORM_TYPE_SAMPLER
} Uniform_Type;



typedef struct Uniform_Config
{
    char name[32];
    u8 size;
    Uniform_Type type;
    Descriptor_Set_Scope scope;
} Uniform_Config;



#pragma once

#include "defines.h"
#include "third_party/cglm/struct.h"

typedef enum resource_type
{
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_STATIC_MESH
} resource_type;

typedef struct Resource
{
    char const* name;
    u32 loader_id;
    void* data;
    u64 data_size;
} Resource;

typedef struct image_resource_data
{
    u8 channel_count;
    u32 width;
    u32 height;
    u8* pixels;
} image_resource_data;

#define TEXTURE_NAME_MAX_LENGTH 128

typedef struct Texture
{
    char name[TEXTURE_NAME_MAX_LENGTH];
    u32 id;
    u32 generation;
    u32 width;
    u32 height;
    u8 channel_count;
    b8 has_transparency;
    void* internal;
} Texture;

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

typedef struct Material_Config
{
    char version[4];
    char name[MAX_MATERIAL_NAME_LENGTH];
    vec4s diffuse_colour;
    char diffuse_texture_name[TEXTURE_NAME_MAX_LENGTH];
    Material_Type type;
    b8 auto_release;
} Material_Config;

typedef struct Material
{
    char name[MAX_MATERIAL_NAME_LENGTH];
    u32 id;
    u32 backend_id;
    u32 generation;
    Material_Type type;
    vec4s diffuse_colour;
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

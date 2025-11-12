#pragma once

#include "defines.h"
#include "third_party/cglm/cglm.h"

typedef enum resource_type {
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_STATIC_MESH
} resource_type;

typedef struct resource {
    char const* name;
    u32 loader_id;
    char* complete_path;
    u64 data_size;
    void* data;
} resource;

typedef struct image_resource_data {
    u8 channel_count;
    u32 width;
    u32 height;
    u8* pixels;
} image_resource_data;

#define TEXTURE_NAME_MAX_LENGTH 512

typedef struct texture_resource {
    u32 id;
    u32 width;
    u32 height;
    u8 channel_count;
    b8 has_transparency;
    u32 generation;
    char name[TEXTURE_NAME_MAX_LENGTH];
    void* internal;
} texture_resource;

typedef enum texture_use {
    TEXTURE_USE_UNKNOWN = 0x00,
    TEXTURE_USE_MAP_DIFFUSE = 0x01
} texture_use;

typedef struct texture_map {
    texture_resource* texture;
    texture_use use;
} texture_map;

#define MATERIAL_NAME_MAX_LENGTH 256

typedef enum material_type {
    MATERIAL_TYPE_WORLD,
    MATERIAL_TYPE_UI
} material_type;

typedef struct material_config {
    char name[MATERIAL_NAME_MAX_LENGTH];
    material_type type;
    b8 auto_release;
    vec4 diffuse_colour;
    char diffuse_map_name[TEXTURE_NAME_MAX_LENGTH];
} material_config;

typedef struct material_resource
{
    u32 id;
    u32 backend_id;
    char name[MATERIAL_NAME_MAX_LENGTH];
    u32 generation;
    material_type type;
    vec4 diffuse_colour;
    texture_map diffuse_map;
} material_resource;

#define GEOMETRY_MAX_NAME_LENGTH 256

/**
 * @brief Represents actual geometry in the world.
 * Typically (but not always, depending on use) paired with a material.
 */
typedef struct geometry_resource {
    u32 id;
    u32 internal_id;
    char name[GEOMETRY_MAX_NAME_LENGTH];
    u32 generation;
    material_resource* material;
} geometry_resource;

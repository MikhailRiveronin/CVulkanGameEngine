#pragma once

#include "defines.h"
#include "resources/resources.h"

typedef enum Resource_Type
{
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_SHADER_CONFIG,
    RESOURCE_TYPE_STATIC_MESH,
    RESOURCE_TYPE_ENUM_COUNT
} Resource_Type;

typedef struct Resource
{
    u8 loader_id;
    char const* path;
    u64 data_size;
    void* data;
} Resource;

typedef struct Resource_Loader
{
    u8 id;
    Resource_Type type;
    b8 (* load)(char const* filename, Resource* resource);
    void (* unload)(Resource* resource);
} Resource_Loader;

b8 resource_system_startup(u64* required_memory, void* block);
void resource_system_shutdown();
b8 resource_system_load(char const* filename, Resource_Type type, Resource* resource);
void resource_system_unload(Resource* resource);

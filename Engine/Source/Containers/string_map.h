#pragma once

#include "defines.h"

typedef struct Resource_Reference
{
    bool is_empty;
    bool in_probe;
    char key[32];
    void* value;
} Resource_Reference;

/**
 * @brief A non-shrinking hash table. The keys are exclusively strings. Open addressing collision resolution. Double probing.
 */
typedef struct String_Map
{
    char type[32];
    u32 size;
    u32 data_size;
    u32 used;
    Resource_Reference* buckets;
} String_Map;

LIB_API String_Map* string_map_create(char const* type, u32 size, u32 data_size);
LIB_API void string_map_destroy(String_Map* map);
LIB_API void string_map_insert(String_Map* map, char const* key, void const* value);
LIB_API void string_map_erase(String_Map* map, char const* key);
LIB_API void* string_map_at(String_Map* map, char const* key);

#define STRING_MAP_CREATE(type, size) string_map_create((#type), (size), sizeof(type))
#define STRING_MAP_AT_AS(map, key, type) *(type*)string_map_at((map), (key))

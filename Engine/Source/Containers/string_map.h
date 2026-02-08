#pragma once

#include "defines.h"
#include "linked_list.h"

/**
 * @brief A hash table in which the keys are exclusively strings and with collision resolution using chaining.
 */
typedef struct String_Map
{
    char type[32];
    u32 size;
    u32 data_size;
    Linked_List** buckets;
} String_Map;

LIB_API String_Map* string_map_create(char const* type, u32 size, u32 data_size);
LIB_API void string_map_destroy(String_Map* map);
LIB_API void string_map_insert(String_Map* map, char const* key, void const* data);
LIB_API void string_map_erase(String_Map* map, char const* key);
LIB_API bool string_map_contains(String_Map* map, char const* key);
LIB_API void* string_map_at(String_Map* map, char const* key);

#define STRING_MAP_CREATE(type, size) string_map_create((#type), (size), sizeof(type))
#define STRING_MAP_AT_AS(map, key, type) *(type*)string_map_at((map), (key))

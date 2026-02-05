#pragma once

#include "defines.h"
#include "linked_list.h"

/**
 * @brief A hash table in which the keys are exclusively strings and with collision resolution using chaining.
 */
typedef struct String_Map
{
    char value_type[32];
    u32 size;
    u32 data_size;
    Linked_List** buckets;
} String_Map;

LIB_API String_Map* string_map_create(char const* value_type, u32 size, u32 data_size);
LIB_API void string_map_destroy(String_Map* map);
LIB_API void string_map_insert(String_Map* map, char const* key, void const* data);
LIB_API void string_map_erase(String_Map* map, char const* key);
LIB_API bool string_map_contains(String_Map* map, char const* key);
LIB_API void* string_map_find(String_Map* map, char const* key);

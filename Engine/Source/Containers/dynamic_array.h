#pragma once

#include "defines.h"

/**
 * @brief A dynamic array which takes copy of the data.
 */
typedef struct Dynamic_Array
{
    char type[32];
    u32 capacity;
    u32 size;
    u32 stride;
    void* data;
} Dynamic_Array;

LIB_API Dynamic_Array* dynamic_array_create(char const* type, u32 stride);
LIB_API void dynamic_array_destroy(Dynamic_Array* array);
LIB_API void dynamic_array_reserve(Dynamic_Array* array, u32 new_cap);
LIB_API void dynamic_array_resize(Dynamic_Array* array, u32 count);
LIB_API void dynamic_array_push_back(Dynamic_Array* array, void const* value);
LIB_API void dynamic_array_pop(Dynamic_Array* array, void* value);
LIB_API void* dynamic_array_at(Dynamic_Array* array, u32 pos);

#define DYNAMIC_ARRAY_CREATE(type) dynamic_array_create((#type), sizeof(type))
#define DYNAMIC_ARRAY_AT_AS(array, pos, type) (*(type*)dynamic_array_at((array), (pos)))

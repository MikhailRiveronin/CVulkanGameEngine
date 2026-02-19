#pragma once

#include "defines.h"

typedef struct Linear_Allocator
{
    u32 size;
    u32 allocated;
    void* memory;
} Linear_Allocator;

LIB_API bool linear_allocator_create(u32 size, Linear_Allocator* allocator);
LIB_API void linear_allocator_destroy(Linear_Allocator* allocator);
LIB_API void* linear_allocator_allocate(Linear_Allocator* allocator, u32 size);
LIB_API void linear_allocator_free(Linear_Allocator* allocator);

#pragma once

#include "defines.h"

typedef struct linear_allocator
{
    u64 tracked_memory;
    u64 allocated_memory;
    void* block;
    b8 owns_memory;
} linear_allocator;

LIB_API void linear_allocator_create(u64 tracked_memory, void* block, linear_allocator* allocator);
LIB_API void linear_allocator_destroy(linear_allocator* allocator);

LIB_API void* linear_allocator_allocate(linear_allocator* allocator, u64 size);
LIB_API void linear_allocator_free_all(linear_allocator* allocator);

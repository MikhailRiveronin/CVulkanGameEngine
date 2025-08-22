#pragma once

#include "defines.h"

typedef enum memory_tag {
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_ENUM_COUNT
} memory_tag;

LIB_API b8 memory_system_startup(u64* memory_size, void* memory);
LIB_API void memory_system_shutdown();

LIB_API void* memory_allocate(u64 size, memory_tag tag);
LIB_API void memory_free(void* ptr, u64 size, memory_tag tag);

LIB_API void* memory_set(void* dest, i32 value, u64 size);
LIB_API void* memory_zero(void* dest, u64 size);
LIB_API void* memory_copy(void* dest, void const* src, u64 size);

LIB_API void memoryPrintUsageStr();
LIB_API u64 memory_allocation_count();

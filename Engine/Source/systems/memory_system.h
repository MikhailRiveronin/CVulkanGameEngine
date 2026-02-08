#pragma once

#include "defines.h"

/**
 * @brief Tags to indicate the usage of memory allocations.
 */
typedef enum memory_tag
{
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
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
    MEMORY_TAG_HASHTABLE,
    MEMORY_TAG_RESOURCE,
    MEMORY_TAG_CONTAINERS,
    MEMORY_TAG_ENUM_COUNT
} memory_tag;

typedef struct memory_system_configuration
{
    /**
     * @brief The amount of tracked memory, in bytes.
     */
    u64 tracked_memory;
} memory_system_configuration;

/**
 * @brief Startup the memory system.
 * @param config The configuration for the system.
 * @return TRUE on success, otherwise FALSE.
 */
LIB_API b8 memory_system_startup(memory_system_configuration config);

/**
 * @brief Shutdown the memory system.
 */
LIB_API void memory_system_shutdown();

/**
 * @brief Allocates _size_ bytes of memory. The allocation is tracked for the _tag_.
 * @param size The size of the block of memory in bytes.
 * @param tag Indicates the usage of the allocation.
 * @return A pointer to the allocated memory or NULL.
 */
LIB_API void* memory_system_allocate(u64 size, memory_tag tag);

/**
 * @brief Frees the _block_ of memory. Untracks _size_ bytes from the _tag_.
 * @param block A block of memory to be freed.
 * @param size The size of the block of memory in bytes.
 * @param tag Indicates the usage of the allocation.
 */
LIB_API void memory_system_free(void* block, u64 size, memory_tag tag);

/**
 * @brief Sets the _block_ of memory to _value_ over the _size_.
 * @param block A block of memory to be set.
 * @param value The value to be set.
 * @param size The size of the block of memory in bytes.
 * @return A pointer to the set block of memory.
 */
LIB_API void* memory_system_set(void* block, i32 value, u64 size);

/**
 * @brief Zeroes out the _block_ of memory.
 * @param block A block of memory to be zeroed out.
 * @param size The size of the block of memory in bytes.
 * @return A pointer to the zeroed out block of memory.
 */
LIB_API void* memory_system_zero(void* block, u64 size);

/**
 * @brief Copies the _size_ bytes of memory at _src_ to _dest_.
 * @param dest A block of memory to copy to.
 * @param src A block of memory to copy from.
 * @param size The size of the block of memory to be copied over.
 * @return A pointer to the copied to block of memory.
 */
LIB_API void* memory_system_copy(void* dest, void const* src, u64 size);

/**
 * @brief Provides a string containing the representation of memory usage, categorized by memory tag.
 */
LIB_API void memory_system_print_usage();

/**
 * @brief Provides the count of allocations.
 * @returns The count of allocations.
 */
LIB_API u64 memory_system_allocation_count();

#pragma once

#include "defines.h"

typedef struct dynamic_allocator
{
    void* internal;
} dynamic_allocator;

/**
 * @brief Creates a dynamic allocator. Must be called twice; once passing NULL to _memory_ to obtain amount of _required_memory_, and a second time passing a pre-allocated block to _memory_.
 * @param required_memory Total memory required, in bytes, including bookkeeping.
 * @param block NULL, or a pre-allocated block of memory.
 * @param tracked_memory The amount of tracked memory, in bytes.
 * @param allocator A pointer to the created allocator.
 * @return TRUE on success, otherwise FALSE.
 */
LIB_API b8 dynamic_allocator_create(u64* required_memory, void* block, u64 tracked_memory, dynamic_allocator* allocator);

/**
 * @brief Destroys a dynamic allocator.
 * @param allocator A pointer to the allocator.
 * @return TRUE on success, otherwise FALSE.
 */
LIB_API b8 dynamic_allocator_destroy(dynamic_allocator* allocator);

/**
 * @brief Allocates _size_ bytes from the allocator.
 * @param allocator A pointer to the allocator.
 * @param size The size in bytes to be allocated.
 * @return A pointer to the allocated memory or NULL.
 */
LIB_API void* dynamic_allocator_allocate(dynamic_allocator* allocator, u64 size);

/**
 * @brief Frees the given block of memory.
 * @param allocator A pointer to the allocator.
 * @param block A block of memory to be freed.
 * @param size The size of the block of memory.
 * @return TRUE on success, otherwise FALSE.
 */
LIB_API b8 dynamic_allocator_free(dynamic_allocator* allocator, void* block, u64 size);

/**
 * @brief Obtains the amount of free space left in the allocator.
 * @param allocator A pointer to the allocator.
 * @return The amount of free space in bytes.
 */
LIB_API u64 dynamic_allocator_free_space(dynamic_allocator* allocator);

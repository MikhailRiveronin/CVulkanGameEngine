#pragma once

#include "defines.h"

typedef struct freelist {
    void* internal;
} freelist;

/**
 * @brief Creates a new freelist.
 * Call twice; once passing NULL to memory to obtain memory requirement,
 * and a second time passing an allocated block to memory.
 *
 * @param required_memory_in_bytes A pointer to how much memory is required for the freelist itself.
 * @param memory NULL, or a pre-allocated block of memory for the freelist.
 * @param size_in_bytes The number of bytes for the freelist to keep track of.
 * @param list A pointer to the created freelist.
 */
void freelist_create(u64* required_memory_in_bytes, void* memory, u32 size_in_bytes, freelist* list);

/**
 * @brief Destroys a freelist.
 *
 * @param list A pointer to the freelist.
 */
void freelist_destroy(freelist* list);

/**
 * @brief Attempts to find a free block of memory of the given size.
 *
 * @param list A pointer to the freelist.
 * @param size The size to allocate.
 * @param offset A pointer to the offset to the allocated memory.
 * @return b8 TRUE if a block of memory was found; otherwise FALSE.
 */
b8 freelist_allocate_block(freelist* list, u32 size, u32* offset);

/**
 * @brief Attempts to free a block of memory at the given offset, and of the given size.
 * Can fail if invalid data is passed.
 *
 * @param list A pointer to the freelist.
 * @param size The size to deallocate.
 * @param offset The offset to free at.
 * @return b8 TRUE if freed successfully; otherwise FALSE.
 */
b8 freelist_free_block(freelist* list, u32 size, u32 offset);

/**
 * @brief Clears the free list.
 *
 * @param list A pointer to the freelist.
 */
void freelist_clear(freelist* list);

/**
 * @brief Returns the amount of free space in this list.
 *
 * @param list A pointer to the freelist.
 * @return The amount of free space in bytes.
 */
u64 freelist_free_space(freelist* list);

#pragma once

#include "defines.h"

typedef struct freelist
{
    void* internal;
} freelist;

/**
 * @brief Creates a freelist. Must be called twice; once passing NULL to _memory_ to obtain amount of _required_memory_, and a second time passing a pre-allocated block to _memory_.
 * @param required_memory Total memory required, in bytes, including bookkeeping.
 * @param block NULL, or a pre-allocated block of memory.
 * @param tracked_memory The amount of tracked memory, in bytes.
 * @param list A pointer to the created freelist.
 */
LIB_API void freelist_create(u64* required_memory, void* block, u32 tracked_memory, freelist* list);

/**
 * @brief Destroys a freelist.
 * @param list A pointer to the freelist.
 */
LIB_API void freelist_destroy(freelist* list);

/**
 * @brief Allocates _size_ bytes from the freelist.
 * @param list A pointer to the freelist.
 * @param size The size in bytes to be allocated.
 * @param offset A pointer to the offset to the allocated memory, in bytes.
 * @return TRUE on success, otherwise FALSE.
 */
LIB_API b8 freelist_allocate(freelist* list, u64 size, u32* offset);

/**
 * @brief Frees _size_ bytes from the freelist at the _offset_.
 *
 * @param list A pointer to the freelist.
 * @param size The size in bytes to be freed.
 * @param offset The offset to the allocated memory.
 * @return TRUE on success, otherwise FALSE.
 */
LIB_API b8 freelist_free(freelist* list, u64 size, u64 offset);

/**
 * @brief Resizes the freelist to the _new_tracked_memory_.
 * @param list A pointer to the freelist.
 * @param new_tracked_memory The new amount of tracked memory, in bytes.
 * @return TRUE on success, otherwise FALSE.
 */
LIB_API b8 freelist_resize(freelist* list, u64 new_tracked_memory);

/**
 * @brief Clears the freelist.
 * @param list A pointer to the freelist.
 */
LIB_API void freelist_clear(freelist* list);

/**
 * @brief Returns the amount of free space in this list.
 * @param list A pointer to the freelist.
 * @return The amount of free space, in bytes.
 */
LIB_API u64 freelist_free_space(freelist* list);

#pragma once

#include "defines.h"
#include "linked_list.h"

/**
 * @brief A first-fit freelist organized as linked list.
 */
typedef struct Freelist
{
    u32 total_size;
    Linked_List* nodes;
} Freelist;

LIB_API Freelist* freelist_create(u32 size);
LIB_API void freelist_destroy(Freelist* list);
LIB_API bool freelist_allocate(Freelist* list, u32 required_size, u32* offset);
LIB_API bool freelist_free(Freelist* list, u32 offset, u32 size);









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

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

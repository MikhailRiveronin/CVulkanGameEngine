#include "freelist.h"

#include "core/logger.h"
#include "core/memory_utils.h"

typedef struct freelist_node {
    u32 offset;
    u32 size;
    struct freelist_node* next;
} freelist_node;

typedef struct internal_state {
    u32 size;
    u32 free_space;
    u32 max_entry_count;
    freelist_node* nodes;
    freelist_node* head;
} internal_state;

// Arbitrarily for now.
#define MAX_ENTRY_COUNT 1024

void invalidate_node(freelist_node* node);
freelist_node* get_empty_node(freelist* list);
void try_coalesce_nodes(freelist* list, freelist_node* previous, freelist_node* new_node);

void freelist_create(u64* required_memory_in_bytes, void* memory, u32 size_in_bytes, freelist* list)
{
    *required_memory_in_bytes = sizeof(internal_state) + (sizeof(freelist_node) * MAX_ENTRY_COUNT);
    if (!memory) {
        return;
    }

    memory_zero(memory, *required_memory_in_bytes);
    internal_state* state = (internal_state*)memory;
    state->size = size_in_bytes;
    state->free_space = state->size;
    state->max_entry_count = MAX_ENTRY_COUNT;
    state->nodes = (freelist_node*)((char*)state + sizeof(internal_state));
    state->head = state->nodes;
    state->head->offset = 0;
    state->head->size = state->size;
    state->head->next = 0;

    for (u32 i = 1; i < state->max_entry_count; ++i) {
        invalidate_node(&state->nodes[i]);
    }

    list->internal = state;
}

void freelist_destroy(freelist* list)
{
    if (list && list->internal) {
        internal_state* state = list->internal;
        memory_zero(list->internal, sizeof(internal_state) + sizeof(freelist_node) * state->max_entry_count);
        list->internal = 0;
    }
}

b8 freelist_allocate_block(freelist* list, u32 size, u32* offset)
{
    if (!list || !list->internal || !offset) {
        return FALSE;
    }

    internal_state* state = list->internal;
    freelist_node* node = state->head;
    freelist_node* previous = 0;
    while (node) {
        if (node->size == size) {
            // Exact match.
            *offset = node->offset;
            if (previous) {
                previous->next = node->next;
            } else {
                // This node is the head.
                state->head = node->next;
            }

            invalidate_node(node);
            state->free_space =- size;
            return TRUE;
        } else if (node->size > size) {
            *offset = node->offset;

            node->size -= size;
            node->offset += size;
            state->free_space =- size;
            return TRUE;
        }

        previous = node;
        node = node->next;
    }

    LOG_WARNING("freelist_allocate_block: No block with enough free space found (requested: %uB, available: %lluB)", size, state->free_space);
    return FALSE;
}

b8 freelist_free_block(freelist* list, u32 size, u32 offset)
{
    if (!list || !list->internal || size == 0) {
        return FALSE;
    }

    internal_state* state = list->internal;
    freelist_node* node = state->head;
    freelist_node* previous = 0;

    while (node) {
        if (node->offset > offset) {
            freelist_node* new_node = get_empty_node(list);
            new_node->size = size;
            new_node->offset = offset;

            if (previous) {
                new_node->next = node;
                previous->next = new_node;
            } else {
                new_node->next = node;
                state->head = new_node;
            }

            state->free_space += size;

            try_coalesce_nodes(list, previous, new_node);
            return TRUE;
        }

        previous = node;
        node = node->next;
    }

    LOG_WARNING("freelist_free_block: Unable to find block to be freed");
    return FALSE;
}

void freelist_clear(freelist* list)
{
    if (!list || !list->internal) {
        return;
    }

    internal_state* state = list->internal;
    for (u32 i = 1; i < state->max_entry_count; ++i) {
        state->nodes[i].offset = INVALID_ID;
        state->nodes[i].size = INVALID_ID;
        state->nodes[i].next = 0;
    }

    state->head->offset = 0;
    state->head->size = state->size;
    state->head->next = 0;
}

u64 freelist_free_space(freelist* list)
{
    internal_state* state = list->internal;
    return state->free_space;
}

void invalidate_node(freelist_node* node)
{
    node->offset = INVALID_ID;
    node->size = INVALID_ID;
    node->next = 0;
}

freelist_node* get_empty_node(freelist* list)
{
    internal_state* state = list->internal;
    for (u32 i = 1; i < state->max_entry_count; ++i) {
        if (state->nodes[i].offset == INVALID_ID) {
            return &state->nodes[i];
        }
    }

    return 0;
}

void try_coalesce_nodes(freelist* list, freelist_node* previous, freelist_node* new_node)
{
    if (new_node->next && new_node->offset + new_node->size == new_node->next->offset) {
        new_node->size += new_node->next->size;
        freelist_node* tmp = new_node->next;
        new_node->next = tmp->next;
        invalidate_node(tmp);
    }

    if (previous && previous->offset + previous->size == new_node->offset) {
        previous->size += new_node->size;
        freelist_node* tmp = new_node;
        previous->next = tmp->next;
        invalidate_node(tmp);
    }
}

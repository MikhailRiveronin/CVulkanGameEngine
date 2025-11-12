#include "freelist.h"

#include "core/logger.h"
#include "systems/memory_system.h"

typedef struct freelist_node
{
    u64 offset;
    u64 tracked_memory;
    struct freelist_node* next;
} freelist_node;

typedef struct freelist_state
{
    u64 tracked_memory;
    freelist_node* head;
    void* nodes;
} freelist_state;

#define MAX_ENTRY_COUNT 1024

static void invalidate_node(freelist_node* node);
static freelist_node* get_empty_node(freelist* list);
static void try_coalesce_nodes(freelist* list, freelist_node* previous, freelist_node* new_node);

void freelist_create(u64* required_memory, void* block, u32 tracked_memory, freelist* list)
{
    if (!required_memory || tracked_memory == 0)
    {
        LOG_ERROR("freelist_create: Invalid input parameters");
        return FALSE;
    }

    u64 state_required_memory = sizeof(freelist_state);
    u64 node_array_required_memory = MAX_ENTRY_COUNT * sizeof(freelist_node);
    *required_memory = state_required_memory + node_array_required_memory;
    if (!block)
    {
        return;
    }

    freelist_state* state = (freelist_state*)block;
    state->tracked_memory = tracked_memory;
    state->head = (freelist_node*)state->nodes;
    state->head->offset = 0;
    state->head->tracked_memory = state->tracked_memory;
    state->head->next = 0;
    state->nodes = (void*)((char*)state + state_required_memory);
    memory_system_zero(state->nodes, node_array_required_memory);
    for (u32 i = 1; i < MAX_ENTRY_COUNT; ++i)
    {
        freelist_node* node = &((freelist_node*)state->nodes)[i];
        invalidate_node(node);
    }

    list->internal = state;
}

void freelist_destroy(freelist* list)
{
    if (!list || !list->internal)
    {
        LOG_ERROR("freelist_destroy: Invalid input parameters");
        return;
    }

    freelist_state* state = list->internal;
    u64 state_required_memory = sizeof(freelist_state);
    u64 node_array_required_memory = MAX_ENTRY_COUNT * sizeof(freelist_node);
    u64 required_memory = state_required_memory + node_array_required_memory;
    memory_system_zero(state, required_memory);

    list->internal = 0;
}

b8 freelist_allocate(freelist* list, u64 size, u32* offset)
{
    if (!list || !list->internal || !offset)
    {
        LOG_ERROR("freelist_allocate: Invalid input parameters");
        return FALSE;
    }

    freelist_state* state = (freelist_state*)list->internal;
    freelist_node* node = state->head;
    freelist_node* previous = 0;
    while (node)
    {
        if (node->tracked_memory >= size)
        {
            *offset = node->offset;
            node->offset += size;
            node->tracked_memory -= size;
            if (node->tracked_memory == 0)
            {
                if (previous)
                {
                    previous->next = node->next;
                }
                else
                {
                    state->head = node->next;
                }

                invalidate_node(node);
            }

            return TRUE;
        }

        previous = node;
        node = node->next;
    }

    LOG_WARNING("freelist_allocate: Failed to find a block of memory with enough free space");
    return FALSE;
}

b8 freelist_free(freelist* list, u64 size, u64 offset)
{
    if (!list || !list->internal || size == 0)
    {
        LOG_ERROR("freelist_free: Invalid input parameters");
        return FALSE;
    }

    freelist_state* state = list->internal;
    freelist_node* node = state->head;
    freelist_node* previous = 0;
    while (node)
    {
        if (node->offset > offset)
        {
            freelist_node* new_node = get_empty_node(list);
            new_node->tracked_memory = size;
            new_node->offset = offset;
            if (previous)
            {
                new_node->next = node;
                previous->next = new_node;
            }
            else
            {
                new_node->next = node;
                state->head = new_node;
            }

            try_coalesce_nodes(list, previous, new_node);
            return TRUE;
        }

        previous = node;
        node = node->next;
    }

    LOG_WARNING("freelist_free: Failed to find block to be freed");
    return FALSE;
}

b8 freelist_resize(freelist* list, u64 new_tracked_memory)
{
    freelist_state* state = (freelist_state*)list->internal;
    if (!list || (new_tracked_memory <= state->tracked_memory))
    {
        LOG_ERROR("freelist_resize: Invalid input parameters");
        return FALSE;
    }

    for (u32 i = 0; i < MAX_ENTRY_COUNT; ++i)
    {
        freelist_node* node = &((freelist_node*)state->nodes)[i];
        if (node->offset == INVALID_ID)
        {
            u64 diff = new_tracked_memory - state->tracked_memory;
            node->tracked_memory += diff;
            return TRUE;
        }
    }

    LOG_WARNING("freelist_resize: Failed to resize freelist");
    return FALSE;
}

void freelist_clear(freelist* list)
{
    if (!list || !list->internal)
    {
        LOG_ERROR("freelist_clear: Invalid input parameters");
        return;
    }

    freelist_state* state = (freelist_state*)list->internal;
    state->head->offset = 0;
    state->head->tracked_memory = state->tracked_memory;
    state->head->next = 0;
    for (u32 i = 1; i < MAX_ENTRY_COUNT; ++i)
    {
        freelist_node* node = &((freelist_node*)state->nodes)[i];
        node->offset = INVALID_ID;
        node->tracked_memory = INVALID_ID;
        node->next = 0;
    }
}

u64 freelist_free_space(freelist* list)
{
    if (!list || !list->internal)
    {
        LOG_ERROR("freelist_free_space: Invalid input parameters");
        return 0;
    }

    freelist_state* state = (freelist_state*)list->internal;
    freelist_node* node = state->head;
    u64 space = 0;
    while (node)
    {
        space += node->tracked_memory;
        node = node->next;
    }

    return space;
}

void invalidate_node(freelist_node* node)
{
    node->offset = INVALID_ID;
    node->tracked_memory = INVALID_ID;
    node->next = 0;
}

freelist_node* get_empty_node(freelist* list)
{
    freelist_state* state = (freelist_state*)list->internal;
    for (u32 i = 0; i < MAX_ENTRY_COUNT; ++i)
    {
        freelist_node* node = &((freelist_node*)state->nodes)[i];
        if (node->offset == INVALID_ID)
        {
            return node;
        }
    }

    return 0;
}

void try_coalesce_nodes(freelist* list, freelist_node* previous, freelist_node* new_node)
{
    if (new_node->next && ((new_node->offset + new_node->tracked_memory) == new_node->next->offset))
    {
        new_node->tracked_memory += new_node->next->tracked_memory;
        freelist_node* tmp = new_node->next;
        new_node->next = tmp->next;
        invalidate_node(tmp);
    }

    if (previous && ((previous->offset + previous->tracked_memory) == new_node->offset))
    {
        previous->tracked_memory += new_node->tracked_memory;
        freelist_node* tmp = new_node;
        previous->next = tmp->next;
        invalidate_node(tmp);
    }
}

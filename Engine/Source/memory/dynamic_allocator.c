#include "dynamic_allocator.h"

#include "containers/freelist.h"
#include "core/logger.h"
#include "systems/memory_system.h"

typedef struct dynamic_allocator_state
{
    u64 tracked_memory;
    freelist allocation_tracker;
    void* allocation_tracker_block;
    void* memory_block;
} dynamic_allocator_state;

b8 dynamic_allocator_create(u64* required_memory, void* block, u64 tracked_memory, dynamic_allocator* allocator)
{
    if (!required_memory || tracked_memory == 0)
    {
        LOG_ERROR("dynamic_allocator_create: Invalid input parameters");
        return FALSE;
    }

    u64 state_required_memory = sizeof(dynamic_allocator_state);
    u64 allocation_tracker_required_memory;
    freelist_create(&allocation_tracker_required_memory, 0, tracked_memory, 0);
    *required_memory = state_required_memory + allocation_tracker_required_memory + tracked_memory;
    if (!block)
    {
        return TRUE;
    }

    dynamic_allocator_state* state = (dynamic_allocator_state*)block;
    state->tracked_memory = tracked_memory;
    state->allocation_tracker_block = (void*)((char*)state + sizeof(*state));
    state->memory_block = (char*)state->allocation_tracker_block + allocation_tracker_required_memory;
    freelist_create(&allocation_tracker_required_memory, state->allocation_tracker_block, tracked_memory, &state->allocation_tracker);
    memory_system_zero(state->memory_block, tracked_memory);
    allocator->internal = state;

    return TRUE;
}

b8 dynamic_allocator_destroy(dynamic_allocator* allocator)
{
    if (!allocator)
    {
        LOG_ERROR("dynamic_allocator_destroy: Invalid input parameters");
        return FALSE;
    }

    dynamic_allocator_state* state = (dynamic_allocator_state*)allocator->internal;
    freelist_destroy(&state->allocation_tracker);
    memory_system_zero(state->memory_block, state->tracked_memory);
    state->tracked_memory = 0;
    allocator->internal = 0;

    return TRUE;
}

void* dynamic_allocator_allocate(dynamic_allocator* allocator, u64 size)
{
    if (!allocator || size == 0)
    {
        LOG_ERROR("dynamic_allocator_allocate: Invalid input parameters");
        return 0;
    }

    dynamic_allocator_state* state = (dynamic_allocator_state*)allocator->internal;
    u64 offset = 0;
    if (freelist_allocate(&state->allocation_tracker, size, &offset))
    {
        void* block = (void*)((char*)state->memory_block + offset);
        return block;
    }

    LOG_ERROR("dynamic_allocator_allocate: Failed to allocate memory");
    return 0;
}

b8 dynamic_allocator_free(dynamic_allocator* allocator, void* block, u64 size)
{
    dynamic_allocator_state* state = allocator->internal;
    if (!allocator || !block || size == 0 || block < state->memory_block || block > (state->memory_block + state->total_size))
    {
        LOG_ERROR("dynamic_allocator_free: Invalid input parameters");
        return FALSE;
    }

    u64 offset = (char*)block - (char*)state->memory_block;
    if (!freelist_free(&state->allocation_tracker, size, offset))
    {
        LOG_ERROR("dynamic_allocator_free: Failed to free memory");
        return FALSE;
    }

    return TRUE;
}

u64 dynamic_allocator_free_space(dynamic_allocator* allocator)
{
    dynamic_allocator_state* state = allocator->internal;
    return freelist_free_space(&state->allocation_tracker);
}

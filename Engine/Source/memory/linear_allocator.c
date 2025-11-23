#include "linear_allocator.h"

#include "core/logger.h"
#include "systems/memory_system.h"

void linear_allocator_create(u64 tracked_memory, void* block, linear_allocator* allocator)
{
    if (tracked_memory == 0)
    {
        LOG_ERROR("linear_allocator_create: Invalid input parameters");
        return;
    }

    allocator->tracked_memory = tracked_memory;
    allocator->allocated_memory = 0;
    if (block)
    {
        allocator->block = block;
        allocator->owns_memory = FALSE;
    }
    else
    {
        allocator->block = memory_system_allocate(tracked_memory, MEMORY_TAG_LINEAR_ALLOCATOR);
        allocator->owns_memory = TRUE;
    }
}

void linear_allocator_destroy(linear_allocator* allocator)
{
    if (allocator) {
        allocator->allocated_memory = 0;
        if (allocator->owns_memory && allocator->memory) {
            memory_free(allocator->memory, allocator->tracked_memory, MEMORY_TAG_LINEAR_ALLOCATOR);
        }

        allocator->memory = 0;
        allocator->tracked_memory = 0;
        allocator->owns_memory = FALSE;
    }
}

void* linear_allocator_allocate(linear_allocator* allocator, u64 size)
{
    if (allocator && allocator->memory) {
        if (allocator->allocated_memory + size > allocator->tracked_memory) {
            u64 remaining = allocator->tracked_memory - allocator->allocated_memory;
            LOG_ERROR("linear_allocator_allocate: Tried to allocate %lluB, only %lluB remaining.", size, remaining);
            return 0;
        }

        void* block = ((u8*)allocator->memory) + allocator->allocated_memory;
        allocator->allocated_memory += size;
        return block;
    }

    LOG_ERROR("linear_allocator_allocate - provided allocator not initialized.");
    return 0;
}

void linear_allocator_free_all(linear_allocator* allocator)
{
    if (allocator && allocator->memory) {
        allocator->allocated_memory = 0;
        memory_zero(allocator->memory, allocator->tracked_memory);
    }
}

#include "linear_allocator.h"

#include "core/logger.h"
#include "systems/memory_system.h"

bool linear_allocator_create(u32 size, Linear_Allocator* allocator)
{
    if (size == 0)
    {
        LOG_FATAL("linear_allocator_create: Invalid parameters");
        return false;
    }

    allocator->size = size;
    allocator->allocated = 0;
    allocator->memory = memory_system_allocate(allocator->size, MEMORY_TAG_LINEAR_ALLOCATOR);
    if (!allocator->memory)
    {
        LOG_FATAL("linear_allocator_create: Failed to allocate memory");
        return false;
    }

    return true;
}

void linear_allocator_destroy(Linear_Allocator* allocator)
{
    if (!allocator)
    {
        LOG_FATAL("linear_allocator_destroy: Invalid parameters");
        return;
    }

    memory_system_free(allocator->memory, allocator->size, MEMORY_TAG_LINEAR_ALLOCATOR);
    allocator->size = 0;
    allocator->allocated = 0;
    allocator->memory = 0;
}

void* linear_allocator_allocate(Linear_Allocator* allocator, u32 size)
{
    if (!allocator || size == 0)
    {
        LOG_FATAL("linear_allocator_allocate: Invalid parameters");
        return 0;
    }

    if (allocator->allocated + size > allocator->size)
    {
        LOG_FATAL("linear_allocator_allocate: Not enough memory");
        return 0;
    }

    void* memory = (char*)allocator->memory + allocator->allocated;
    allocator->allocated += size;
    return memory;
}

void linear_allocator_free(Linear_Allocator* allocator)
{
    if (!allocator)
    {
        LOG_FATAL("linear_allocator_free: Invalid parameters");
        return;
    }

    memory_system_zero(allocator->memory, allocator->size);
    allocator->allocated = 0;
}

#include "memory_system.h"

#include "logger.h"
#include "memory/dynamic_allocator.h"
#include "platform/platform.h"

// #include <stdio.h>
// #include <string.h>

typedef struct memory_stats
{
    u64 allocated_memory;
    u64 allocated_memory_by_tags[MEMORY_TAG_ENUM_COUNT];
} memory_stats;

typedef struct memory_system_state
{
    memory_system_configuration config;
    memory_stats stats;

    u64 allocation_count;
    u64 allocator_required_memory;
    dynamic_allocator allocator;
    void* allocator_block;
} memory_system_state;

static memory_system_state* state;

static char const* memory_tag_strs[] = {
    "UNKNOWN     ",
    "ARRAY       ",
    "LINEAR_ALLOC",
    "DARRAY      ",
    "DICT        ",
    "RING_QUEUE  ",
    "BST         ",
    "STRING      ",
    "APPLICATION ",
    "JOB         ",
    "TEXTURE     ",
    "MAT_INST    ",
    "RENDERER    ",
    "GAME        ",
    "TRANSFORM   ",
    "ENTITY      ",
    "ENTITY_NODE ",
    "SCENE       " };

b8 memory_system_startup(memory_system_configuration config)
{
    if (config.tracked_memory == 0)
    {
        LOG_ERROR("memory_system_startup: Invalid input parameters");
        return FALSE;
    }

    u64 state_required_memory = sizeof(*state);
    u64 allocator_required_memory = 0;
    dynamic_allocator_create(&allocator_required_memory, 0, config.tracked_memory, 0);
    void* block = platform_allocate(state_required_memory + allocator_required_memory, FALSE);
    if (!block)
    {
        LOG_FATAL("memory_system_startup: Failed to allocate required memory");
        return FALSE;
    }

    state = (memory_system_state*)block;
    state->config = config;
    state->allocation_count = 0;
    state->allocator_required_memory = allocator_required_memory;
    state->allocator_block = (void*)((char*)block + state_required_memory);
    platform_zero_memory(&state->stats, sizeof(state->stats));
    if (!dynamic_allocator_create(&state->allocator_required_memory, state->allocator_block, config.tracked_memory, &state->allocator))
    {
        LOG_FATAL("memory_system_startup: Failed to create internal dynamic allocator");
        return FALSE;
    }

    LOG_DEBUG("memory_system_startup: Memory system successfully allocated %llu bytes", config.tracked_memory);
    return TRUE;
}

void memory_system_shutdown()
{
    if (state)
    {
        dynamic_allocator_destroy(&state->allocator);
        platform_free(state, FALSE);
    }

    state = 0;
}

void* memory_system_allocate(u64 size, memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN)
    {
        LOG_WARNING("memory_system_allocate: Called with MEMORY_TAG_UNKNOWN");
    }

    if (state)
    {
        state->stats.allocated_memory += size;
        state->stats.allocated_memory_by_tags[tag] += size;
        state->allocation_count++;
        void* block = dynamic_allocator_allocate(&state->allocator, size);
        if (!block)
        {
            LOG_FATAL("memory_system_allocate: Failed to allocate required memory");
            return 0;
        }

        platform_zero_memory(block, size);
        return block;
    }

    LOG_WARNING("memory_system_allocate: Called before the system is initialized");
    return 0;
}

void memory_system_free(void* block, u64 size, memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN)
    {
        LOG_WARNING("memory_system_free: Called with MEMORY_TAG_UNKNOWN");
    }

    if (state)
    {
        state->stats.allocated_memory -= size;
        state->stats.allocated_memory_by_tags[tag] -= size;
        if (!dynamic_allocator_free(&state->allocator, block, size))
        {
            LOG_FATAL("memory_system_free: Failed to free the block of memory");

            // TODO: Report error
            return;
        }
    }

    LOG_WARNING("memory_system_free: Called before the system is initialized");
}

void* memory_system_set(void* dest, i32 value, u64 size)
{
    return platform_set_memory(dest, value, size);
}

void* memory_system_zero(void* dest, u64 size)
{
    return memory_system_set(dest, 0, size);
}

void* memory_system_copy(void* dest, void const* src, u64 size)
{
    return platform_copy_memory(dest, src, size);
}

void memory_system_print_usage()
{
    char buffer[8192];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Tagged memory allocations:\n");
    for (u32 i = 0; i < MEMORY_TAG_ENUM_COUNT; ++i)
    {
        if (state->stats.allocated_memory_by_tags[i] > GIBIBYTES(1))
        {
            sprintf(buffer + strlen(buffer), "    %.2s: %fGiB\n", memory_tag_strs[i], state->stats.allocated_memory_by_tags[i] / (float)GIBIBYTES(1));
        }
        else if (state->stats.allocated_memory_by_tags[i] > MEBIBYTES(1))
        {
            sprintf(buffer + strlen(buffer), "    %s: %.2fMiB\n", memory_tag_strs[i], state->stats.allocated_memory_by_tags[i] / (float)MEBIBYTES(1));
        }
        else if (state->stats.allocated_memory_by_tags[i] > KIBIBYTES(1))
        {
            sprintf(buffer + strlen(buffer), "    %s: %.2fKiB\n", memory_tag_strs[i], state->stats.allocated_memory_by_tags[i] / (float)KIBIBYTES(1));
        }
        else
        {
            sprintf(buffer + strlen(buffer), "    %s: %lluB\n", memory_tag_strs[i], state->stats.allocated_memory_by_tags[i]);
        }
    }

    LOG_DEBUG("%s", buffer);
}

u64 memory_system_allocation_count()
{
    if (state)
    {
        return state->allocation_count;
    }

    LOG_WARNING("memory_system_allocation_count: Called before the system is initialized");
    return 0;
}

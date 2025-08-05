#include "memory.h"
#include "logger.h"
#include "Platform/Platform.h"

#include <stdio.h>
#include <string.h>

static char const* memoryTagStrings[] = {
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

typedef struct memory_stats {
    u64 allocatedTotal;
    u64 allocatedTagged[MEMORY_TAG_ENUM_COUNT];
} memory_stats;

typedef struct memory_system_state {
    memory_stats stats;
    u64 allocation_count;
} memory_system_state;

static memory_system_state* state;

void memory_init(u64* required_memory_size, void* memory)
{
    *required_memory_size = sizeof(*state);
    if (!memory) {
        return;
    }

    state = memory;
    state->allocation_count = 0;
    platform_zero_memory(&state->stats, sizeof(state->stats));
}

void memory_destroy(void* memory)
{
    state = 0;
}

void* memory_allocate(u64 size, MemoryTag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN) {
        LOG_WARNING("allocate() called using MEMORY_TAG_UNKNOWN. Use another allocation class");
    }

    if (state) {
        state->stats.allocatedTotal += size;
        state->stats.allocatedTagged[tag] += size;
        state->allocation_count++;
    }

    // TODO: Memory alignment.
    void* ptr = platformAllocate(size, FALSE);
    platform_zero_memory(ptr, size);
    return ptr;
}

void memory_free(void* ptr, u64 size, MemoryTag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN) {
        LOG_WARNING("deallocate() called using MEMORY_TAG_UNKNOWN. Use another allocation class");
    }

    state->stats.allocatedTotal -= size;
    state->stats.allocatedTagged[tag] -= size;

    // TODO: Memory alignment.
    platformFree(ptr, FALSE);
}

void* memorySet(void* dest, i32 value, u64 size)
{
    return platformSetMemory(dest, value, size);
}

void* memory_zero(void* dest, u64 size)
{
    return memorySet(dest, 0, size);
}

void* memoryCopy(void* dest, void const* src, u64 size)
{
    return platformCopyMemory(dest, src, size);
}

void memoryPrintUsageStr()
{
    u32 const KiB = 1024;
    u32 const MiB = KiB * KiB;
    u32 const GiB = KiB * KiB * KiB;

    char buffer[8192];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Tagged memory allocations:\n");

    for (u32 i = 0; i < MEMORY_TAG_ENUM_COUNT; ++i) {
        if (state->stats.allocatedTagged[i] > GiB) {
            sprintf(buffer + strlen(buffer), "    %.2s: %fGiB\n",
                memoryTagStrings[i],
                state->stats.allocatedTagged[i] / (float)GiB);
        }
        else if (state->stats.allocatedTagged[i] > MiB) {
            sprintf(buffer + strlen(buffer), "    %s: %.2fMiB\n",
                memoryTagStrings[i],
                state->stats.allocatedTagged[i] / (float)MiB);
        }
        else if (state->stats.allocatedTagged[i] > KiB) {
            sprintf(buffer + strlen(buffer), "    %s: %.2fKiB\n",
                memoryTagStrings[i],
                state->stats.allocatedTagged[i] / (float)KiB);
        }
        else {
            sprintf(buffer + strlen(buffer), "    %s: %lluB\n",
                memoryTagStrings[i],
                state->stats.allocatedTagged[i]);
        }
    }
    LOG_DEBUG("%s", buffer);
}

u64 memory_allocation_count()
{
    if (state) {
        return state->allocation_count;
    }
    return 0;
}

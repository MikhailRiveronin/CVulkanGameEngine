#include "Memory.h"
#include "Logger.h"
#include "Platform/Platform.h"

#include <stdio.h>
#include <string.h>

typedef struct MemoryState {
    u64 allocatedTotal;
    u64 allocatedTagged[MEMORY_TAG_ENUM_COUNT];
} MemoryState;

static MemoryState memoryState;

static char const* memoryTagStrings[] = {
    "UNKNOWN    ",
    "ARRAY      ",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      " };

void memoryInit()
{
    platformZeroMemory(&memoryState, sizeof(memoryState));
}

void memoryDestroy()
{
}

void* allocate(u64 size, MemoryTag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN) {
        LOG_WARNING("allocate() called using MEMORY_TAG_UNKNOWN. Use another allocation class");
    }

    memoryState.allocatedTotal += size;
    memoryState.allocatedTagged[tag] += size;

    // TODO: Memory alignment.
    void* ptr = platformAllocate(size, FALSE);
    platformZeroMemory(ptr, size);
    return ptr;
}

void deallocate(void* ptr, u64 size, MemoryTag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN) {
        LOG_WARNING("deallocate() called using MEMORY_TAG_UNKNOWN. Use another allocation class");
    }

    memoryState.allocatedTotal -= size;
    memoryState.allocatedTagged[tag] -= size;

    // TODO: Memory alignment.
    platformFree(ptr, FALSE);
}

void* memorySet(void* dest, i32 value, u64 size)
{
    return platformSetMemory(dest, value, size);
}

void* memoryZero(void* dest, u64 size)
{
    return memorySet(dest, 0, size);
}

void* memoryCopy(void* dest, void const* src, u64 size)
{
    return platformCopyMemory(dest, src, size);
}

void memoryPrintUsageStr()
{
    const u32 KiB = 1024;
    const u32 MiB = KiB * KiB;
    const u32 GiB = KiB * KiB * KiB;

    char buffer[8192];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Tagged memory allocations:\n");

    for (u32 i = 0; i < MEMORY_TAG_ENUM_COUNT; ++i) {
        if (memoryState.allocatedTagged[i] > GiB) {
            sprintf(buffer + strlen(buffer), "    %.2s: %fGiB\n",
                memoryTagStrings[i],
                memoryState.allocatedTagged[i] / (float)GiB);
        }
        else if (memoryState.allocatedTagged[i] > MiB) {
            sprintf(buffer + strlen(buffer), "    %s: %.2fMiB\n",
                memoryTagStrings[i],
                memoryState.allocatedTagged[i] / (float)MiB);
        }
        else if (memoryState.allocatedTagged[i] > KiB) {
            sprintf(buffer + strlen(buffer), "    %s: %.2fKiB\n",
                memoryTagStrings[i],
                memoryState.allocatedTagged[i] / (float)KiB);
        }
        else {
            sprintf(buffer + strlen(buffer), "    %s: %lluB\n",
                memoryTagStrings[i],
                memoryState.allocatedTagged[i]);
        }
    }
    LOG_DEBUG("%s", buffer);
}

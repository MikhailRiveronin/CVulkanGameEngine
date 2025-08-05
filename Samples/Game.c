#include "game.h"
#include <Core/logger.h>
#include <Core/memory.h>
#include <Core/Input.h>

b8 gameOnInit(struct Game* game)
{
    return TRUE;
}

b8 gameOnUpdate(struct Game* game, f64 deltaTime)
{
    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = memory_allocation_count();
    if (input_is_key_up('M') && input_was_key_down('M')) {
        LOG_DEBUG("Allocations: %llu (%llu this frame)", alloc_count, alloc_count - prev_alloc_count);
    }

    return TRUE;
}

b8 gameOnRender(struct Game* game, f64 deltaTime)
{
    return TRUE;
}

void gameOnResize(struct Game* game, i32 width, i32 height)
{
}

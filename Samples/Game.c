#include "game.h"
#include <core/logger.h>
#include <core/memory.h>
#include <core/input.h>

b8 gameOnInit(struct game_instance* game)
{
    return TRUE;
}

b8 gameOnUpdate(struct game_instance* game, f64 delta_time)
{
    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = memory_allocation_count();
    if (input_is_key_up('M') && input_was_key_down('M')) {
        LOG_DEBUG("Allocations: %llu (%llu this frame)", alloc_count, alloc_count - prev_alloc_count);
    }

    return TRUE;
}

b8 gameOnRender(struct game_instance* game, f64 delta_time)
{
    return TRUE;
}

void gameOnResize(struct game_instance* game, i32 width, i32 height)
{
}

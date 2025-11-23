#include "clock.h"
#include "platform/platform.h"

void clock_update(clock* clock)
{
    if (clock->start != 0) {
        clock->elapsed = platform_get_absolute_time() - clock->start;
    }
}

void clock_start(clock* clock)
{
    clock->start = platform_get_absolute_time();
    clock->elapsed = 0;
}

void clock_stop(clock* clock)
{
    clock->start = 0;
}

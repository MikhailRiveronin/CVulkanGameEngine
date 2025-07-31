#include "Clock.h"
#include "Platform/Platform.h"

void clockUpdate(Clock* clock)
{
    if (clock->start != 0) {
        clock->elapsed = platform_get_absolute_time() - clock->start;
    }
}

void clockStart(Clock* clock)
{
    clock->start = platform_get_absolute_time();
    clock->elapsed = 0;
}

void clockStop(Clock* clock)
{
    clock->start = 0;
}

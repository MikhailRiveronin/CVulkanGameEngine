#include "Clock.h"
#include "Platform/Platform.h"

void clockUpdate(Clock* clock)
{
    if (clock->start != 0) {
        clock->elapsed = platformGetAbsoluteTime() - clock->start;
    }
}

void clockStart(Clock* clock)
{
    clock->start = platformGetAbsoluteTime();
    clock->elapsed = 0;
}

void clockStop(Clock* clock)
{
    clock->start = 0;
}

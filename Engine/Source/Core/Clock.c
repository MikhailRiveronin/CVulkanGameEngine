#include "Clock.h"
#include "Platform/Platform.h"

void clockUpdate(Clock* clock)
{
    if (clock->startTime != 0) {
        clock->elapsedTime = platformGetAbsoluteTime() - clock->startTime;

    }
}

void clockStart(Clock* clock)
{
    clock->startTime = platformGetAbsoluteTime();
    clock->elapsedTime = 0;
}

void clockStop(Clock* clock)
{
    clock->startTime = 0;
}

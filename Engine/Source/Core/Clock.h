#pragma once

#include "Defines.h"

typedef struct Clock {
    f64 startTime;
    f64 elapsedTime;
} Clock;

// Should be called just before checking elapsed time.
void clockUpdate(Clock* clock);

// Also resets elapsed time.
void clockStart(Clock* clock);

// Does not reset elapsed time.
void clockStop(Clock* clock);

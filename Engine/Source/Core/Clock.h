#pragma once

#include "Defines.h"

typedef struct Clock {
    f64 start;
    f64 elapsed;
} Clock;

// Should be called just before checking elapsed time.
void clockUpdate(Clock* clock);

// Also resets elapsed time.
void clockStart(Clock* clock);

// Does not reset elapsed time.
void clockStop(Clock* clock);

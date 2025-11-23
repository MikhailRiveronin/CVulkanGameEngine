#pragma once

#include "defines.h"

typedef struct clock {
    f64 start;
    f64 elapsed;
} clock;

// Should be called just before checking elapsed time.
LIB_API void clock_update(clock* clock);

// Also resets elapsed time.
LIB_API void clock_start(clock* clock);

// Does not reset elapsed time.
LIB_API void clock_stop(clock* clock);

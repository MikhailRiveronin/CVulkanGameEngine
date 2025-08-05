#include "logger.h"
#include "Platform/Platform.h"

// TODO: Temporary solution.
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static char const* level_strings[] = {
    "[FATAL]:   ",
    "[ERROR]:   ",
    "[WARNING]: ",
    "[INFO]:    ",
    "[DEBUG]:   ",
    "[TRACE]:   " };

typedef struct logging_system_state {
    b8 initialized;
} logging_system_state;

static logging_system_state* state;

b8 logger_init(u64* required_memory_size, void* memory)
{
    *required_memory_size = sizeof(*state);
    if (!memory) {
        return TRUE;
    }

    state = memory;
    state->initialized = TRUE;

    // TODO: Create log file.
    return TRUE;
}

void logger_destroy(void* memory)
{
    // TODO: Write to log file.
    state = 0;
}

void logOutput(LogLevel level, char const* message, ...)
{
    char buffer[8192];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s", level_strings[level]);

    va_list vaList;
    va_start(vaList, message);
    vsprintf(buffer + strlen(buffer), message, vaList);
    va_end(vaList);

    char output[8192];
    sprintf(output, "%s\n", buffer);
    if (level < LOG_LEVEL_WARNING) {
        platformWriteConsoleError(output, level);
    }
    else {
        platformWriteConsoleOutput(output, level);
    }
}

#include "logger.h"
#include "platform/platform.h"

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

typedef struct logger_system_state {
    b8 initialized;
} logger_system_state;
static logger_system_state* system_state;

b8 logger_system_startup(u64* required_memory, void* memory)
{
    *required_memory = sizeof(*system_state);
    if (!memory) {
        return TRUE;
    }

    system_state = memory;
    system_state->initialized = TRUE;

    // TODO: Create log file.
    return TRUE;
}

void logger_system_shutdown(void* memory)
{
    system_state = 0;
    // TODO: Write to log file.
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

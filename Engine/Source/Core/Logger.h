#pragma once

#include "Defines.h"

#define LOG_WARNING_ENABLED
#define LOG_INFO_ENABLED
#ifdef _DEBUG
#define LOG_DEBUG_ENABLED
#define LOG_TRACE_ENABLED
#endif

typedef enum LogLevel {
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE
} LogLevel;

b8 loggerInit();
void loggerDestroy();

API void logOutput(LogLevel level, char const* message, ...);

#define LOG_FATAL(message, ...) logOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...) logOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__)
#define LOG_WARNING(message, ...) logOutput(LOG_LEVEL_WARNING, message, ##__VA_ARGS__)
#define LOG_INFO(message, ...) logOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__)
#ifdef LOG_DEBUG_ENABLED
#define LOG_DEBUG(message, ...) logOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#else
#define LOG_DEBUG(message, ...)
#endif
#ifdef LOG_TRACE_ENABLED
#define LOG_TRACE(message, ...) logOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#else
#define LOG_TRACE(message, ...)
#endif

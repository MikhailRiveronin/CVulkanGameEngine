#pragma once

#include "defines.h"

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

/**
 * @brief Initializes logging system. Call twice; once with state = 0 to get required memory size,
 * then a second time passing allocated memory to state.
 * 
 * @param required_memory_size A pointer to hold the required memory size of internal state.
 * @param memory 0 if just requesting memory requirement, otherwise allocated block of memory.
 * @return b8 True on success; otherwise false.
 */
b8 logger_init(u64* required_memory_size, void* memory);
void logger_destroy(void* memory);

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

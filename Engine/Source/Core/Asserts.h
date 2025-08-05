#pragma once

#include "defines.h"
#include "logger.h"

#ifdef _DEBUG
#define ASSERT_ENABLED
#endif

#ifdef ASSERT_ENABLED
#if _MSC_VER
#include <intrin.h>
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK() __builtin_trap()
#endif

#define ASSERT(expr)                                                              \
    do {                                                                          \
        if (!(expr)) {                                                            \
            LOG_FATAL("Assertion failed: %s: %d: %s", __FILE__, __LINE__, #expr); \
            DEBUG_BREAK();                                                        \
        }                                                                         \
    }                                                                             \
    while (0)

#define ASSERT_MSG(expr, message)                                                              \
    do {                                                                                       \
        if (!(expr)) {                                                                         \
            LOG_FATAL("Assertion failed: %s: %d: %s: %s", __FILE__, __LINE__, #expr, message); \
            DEBUG_BREAK();                                                                     \
        }                                                                                      \
    }                                                                                          \
    while (0)
#endif

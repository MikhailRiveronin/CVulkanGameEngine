#pragma once

#include <stdint.h>

// Types.
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef _Bool b8;
typedef u32 b32;

#define TRUE 1
#define FALSE 0

// Library API.
#ifdef EXPORT
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif

#define MAX(a, b) ((a) > (b)) ? (a) : (b)
#define CLAMP(value, min, max) ((value) < (min)) ? (min) : ((value) > (max)) ? (max) : (value)

// Inlining
#ifdef _MSC_VER
#define KINLINE __forceinline
#define KNOINLINE __declspec(noinline)
#else
#define KINLINE static inline
#define KNOINLINE
#endif

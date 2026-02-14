#pragma once

#include <vulkan/vulkan.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


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
// typedef _Bool b8;
typedef u32 b32;

#define TRUE 1
#define FALSE 0

#define INVALID_ID 4294967295u

#ifdef EXPORT
#define LIB_API __declspec(dllexport)
#else
#define LIB_API __declspec(dllimport)
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

/** @brief Gets the number of bytes from amount of gibibytes (GiB) (1024 * 1024 * 1024) */
#define GIBIBYTES(amount) amount * 1024 * 1024 * 1024
/** @brief Gets the number of bytes from amount of mebibytes (MiB) (1024 * 1024) */
#define MEBIBYTES(amount) amount * 1024 * 1024
/** @brief Gets the number of bytes from amount of kibibytes (KiB) (1024) */
#define KIBIBYTES(amount) amount * 1024

/** @brief Gets the number of bytes from amount of gigabytes (GB) (1000 * 1000 * 1000) */
#define GIGABYTES(amount) amount * 1000 * 1000 * 1000
/** @brief Gets the number of bytes from amount of megabytes (MB) (1000 * 1000) */
#define MEGABYTES(amount) amount * 1000 * 1000
/** @brief Gets the number of bytes from amount of kilobytes (KB) (1000) */
#define KILOBYTES(amount) amount * 1000



/**
 *  @brief A memory range.
 */
typedef struct Memory_Range
{
    u64 offset;
    u64 size;
} Memory_Range;

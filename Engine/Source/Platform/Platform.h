#pragma once

#include "defines.h"
#include "containers/darray.h"

typedef struct platform_state {
    void* specific;
} platform_state;

b8 platform_system_startup(
    u64* memory_size,
    void* memory,
    platform_state* plat_state,
    char const* appName,
    i32 x, i32 y,
    i32 width, i32 height);
void platform_system_shutdown(platform_state* plat_state);

b8 platformProcMessages(platform_state* plat_state);

void* platform_allocate(u64 size, b8 aligned);
void platform_free(void* ptr, b8 aligned);

void* platform_set_memory(void* dest, i32 value, u64 size);
void* platform_zero_memory(void* dest, u64 size);
void* platform_copy_memory(void* dest, void const* src, u64 size);

void platformWriteConsoleOutput(char const* message, u8 color);
void platformWriteConsoleError(char const* message, u8 color);

f64 platform_get_absolute_time();
void platformSleep(u64 ms);

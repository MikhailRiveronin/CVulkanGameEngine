#pragma once

#include "Defines.h"

typedef struct PlatformState
{
    void *state;
} PlatformState;

b8 platformInit(PlatformState *platformState, const char *appName, i32 x, i32 y, i32 width, i32 height);
void platformTerminate(PlatformState *platformState);
b8 platformProcMessages(PlatformState *platformState);

void *platformAllocate(u64 size, b8 isAligned);
void platformFree(void *ptr, b8 isAligned);
void *platformZeroMemory(void *ptr, u64 size);
void *platformSetMemory(void *ptr, i32 value, u64 size);
void *platformCopyMemory(void *dest, const void *src, u64 size);

void platformWriteConsole(const char *message, b8 isError);
void platformWriteError(const char *message);

f64 platformGetTime();
void platformSleep(u64 ms);

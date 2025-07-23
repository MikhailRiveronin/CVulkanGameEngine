#pragma once

#include "Defines.h"

typedef struct PlatformState {
    void* specific;
} PlatformState;

b8 platformInit(PlatformState* platformState, char const* appName, i32 x, i32 y, i32 width, i32 height);
void platformDestroy(PlatformState* platformState);

b8 platformProcMessages(PlatformState* platformState);

void* platformAllocate(u64 size, b8 aligned);
void platformFree(void* ptr, b8 aligned);

void* platformSetMemory(void* dest, i32 value, u64 size);
void* platformZeroMemory(void* dest, u64 size);
void* platformCopyMemory(void* dest, void const* src, u64 size);

void platformWriteConsoleOutput(char const* message, u8 color);
void platformWriteConsoleError(char const* message, u8 color);

f64 platformGetAbsoluteTime();
void platformSleep(u64 ms);

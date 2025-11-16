#pragma once

#include "defines.h"
#include "core/asserts.h"
#include "systems/memory_system.h"

#define DARRAY_MIN_CAPACITY 1
#define DARRAY_EXPAND_FACTOR 2

#define DARRAY(type)      \
    struct {              \
        u64 capacity;     \
        u64 size;         \
        type* data;       \
        memory_tag memory; \
    }

#define DARRAY_INIT(array, memoryTag)                                                            \
    do {                                                                                         \
        (array).data = memory_system_allocate(DARRAY_MIN_CAPACITY * sizeof(*(array).data), (memoryTag)); \
        if ((array).data) {                                                                      \
            memory_system_zero((array).data, DARRAY_MIN_CAPACITY * sizeof(*(array).data));               \
            (array).capacity = DARRAY_MIN_CAPACITY;                                              \
            (array).size = 0;                                                                    \
            (array).memory = (memoryTag);                                                        \
        }                                                                                        \
        else {                                                                                   \
            (array).data = NULL;                                                                 \
            (array).capacity = 0;                                                                \
            (array).size = 0;                                                                    \
            (array).memory = MEMORY_TAG_UNKNOWN;                                                 \
        }                                                                                        \
    }                                                                                            \
    while (0)

#define DARRAY_EXPAND(array, count)                                                                 \
    do {                                                                                            \
        u64 newCapacity = MAX(count, DARRAY_MIN_CAPACITY);                                          \
        if (newCapacity != DARRAY_MIN_CAPACITY) {                                                   \
            void* newData = memory_system_allocate(newCapacity * sizeof(*(array).data), (array).memory);    \
            if (newData) {                                                                          \
                memory_system_zero(newData, newCapacity * sizeof(*(array).data));                           \
                memory_system_copy(newData, (array).data, (array).size * sizeof(*(array).data));            \
                memory_system_free((array).data, (array).capacity * sizeof(*(array).data), (array).memory); \
                (array).data = newData;                                                             \
                (array).capacity = newCapacity;                                                     \
            }                                                                                       \
            else {                                                                                  \
                ASSERT(FALSE);                                                                      \
            }                                                                                       \
        }                                                                                           \
    }                                                                                               \
    while (0)

#define DARRAY_DEFINE(type, array, count, memoryTag) \
    DARRAY(type) array;                              \
    DARRAY_INIT(array, memoryTag);                   \
    DARRAY_EXPAND(array, count);                     \
    (array).size = count;

#define DARRAY_RESERVE(array, count, memoryTag) \
    DARRAY_INIT(array, memoryTag);              \
    DARRAY_EXPAND(array, count)

#define DARRAY_DESTROY(array)                                                                   \
    do {                                                                                        \
        if ((array).data) {                                                                     \
            memory_system_free((array).data, (array).capacity * sizeof(*(array).data), (array).memory); \
            (array).data = NULL;                                                                \
        }                                                                                       \
        (array).capacity = 0;                                                                   \
        (array).size = 0;                                                                       \
    }                                                                                           \
    while (0)

#define DARRAY_PUSH(array, ...)                                     \
    do {                                                            \
        if ((array).size == (array).capacity) {                     \
            u64 capacity = (array).capacity * DARRAY_EXPAND_FACTOR; \
            DARRAY_EXPAND(array, capacity);                         \
        }                                                           \
        (array).data[(array).size++] = __VA_ARGS__;                 \
    }                                                               \
    while (0)

#define DARRAY_POP(array, value)                      \
    do {                                              \
        if ((array).size > 0) {                       \
            (value) = (array).data[(array).size - 1]; \
            (array).size--;                           \
        }                                             \
        else {                                        \
            ASSERT(FALSE);                            \
        }                                             \
    }                                                 \
    while (0)

#define DARRAY_INSERT(array, pos, ...)                                                                                \
    do {                                                                                                              \
        if (pos >= 0 && pos < (array).size) {                                                                         \
            if ((array).size == (array).capacity) {                                                                   \
                DARRAY_EXPAND((array), (array).capacity * DARRAY_EXPAND_FACTOR);                                      \
            }                                                                                                         \
            if (pos != (array).size - 1) {                                                                            \
                memory_system_copy((array).data + pos + 1, (array).data + pos, ((array).size - pos) * sizeof(*(array).data)); \
            }                                                                                                         \
            (array).data[(array).size++] = __VA_ARGS__;                                                               \
        }                                                                                                             \
        else {                                                                                                        \
            ASSERT(FALSE);                                                                                            \
        }                                                                                                             \
    }                                                                                                                 \
    while (0)

#define DARRAY_ERASE(array, pos, value)                                                                                   \
    do {                                                                                                                  \
        if ((array).size > 0 && pos >= 0 && pos < (array).size) {                                                         \
            (value) = (array).data[pos];                                                                                  \
            if (pos != (array).size - 1) {                                                                                \
                memory_system_copy((array).data + pos, (array).data + pos + 1, ((array).size - pos - 1) * sizeof(*(array).data)); \
            }                                                                                                             \
            (array).size--;                                                                                               \
        }                                                                                                                 \
        else {                                                                                                            \
            ASSERT(FALSE);                                                                                                \
        }                                                                                                                 \
    }                                                                                                                     \
    while (0)

#define DARRAY_CLEAR(array) \
    do {                    \
        (array).size = 0;   \
    }                       \
    while (0)

#define DARRAY_AT(array, pos) (array).data[(pos)]

typedef DARRAY(char const*) DARRAY_CSTRING;

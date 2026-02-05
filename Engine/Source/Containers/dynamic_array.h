#pragma once

#include "defines.h"
#include "core/asserts.h"
#include "systems/memory_system.h"

typedef struct Dynamic_Array
{
    u64 capacity;
    u64 size;
    u64 stride;
    void* data;
} Dynamic_Array;

#define DYNAMIC_ARRAY_CREATE(type, array)                                                    \
    do                                                                                       \
    {                                                                                        \
        (array) = memory_system_allocate(sizeof(*array), MEMORY_TAG_DYNAMIC_ARRAY);          \
        if ((array))                                                                         \
        {                                                                                    \
            (array)->capacity = 1;                                                           \
            (array)->size = 0;                                                               \
            (array)->stride = sizeof(type);                                                  \
            (array)->data = memory_system_allocate(array->stride, MEMORY_TAG_DYNAMIC_ARRAY); \
        }                                                                                    \
        else                                                                                 \
        {                                                                                    \
            LOG_FATAL("DYNAMIC_ARRAY_CREATE: Fatal error");                                  \
            ASSERT(FALSE);                                                                   \
        }                                                                                    \
    }                                                                                        \
    while (0)

#define DYNAMIC_ARRAY_DESTROY(array)                                                                              \
    do                                                                                                            \
    {                                                                                                             \
        if ((array))                                                                                              \
        {                                                                                                         \
            if ((array)->data)                                                                                    \
            {                                                                                                     \
                memory_system_free((array)->data, (array)->capacity * (array)->stride, MEMORY_TAG_DYNAMIC_ARRAY); \
            }                                                                                                     \
            memory_system_free((array), sizeof(*(array)), MEMORY_TAG_DYNAMIC_ARRAY);                              \
        }                                                                                                         \
    }                                                                                                             \
    while (0)

#define DYNAMIC_ARRAY_RESERVE(array, new_cap)                                                                         \
    do                                                                                                                \
    {                                                                                                                 \
        if ((array))                                                                                                  \
        {                                                                                                             \
            if ((new_cap) > (array)->capacity)                                                                        \
            {                                                                                                         \
                void* new_data = memory_system_allocate((new_cap) * (array)->stride, MEMORY_TAG_DYNAMIC_ARRAY);       \
                if (new_data)                                                                                         \
                {                                                                                                     \
                    memory_system_copy(new_data, (array)->data, (array)->size * (array)->stride);                     \
                    memory_system_free((array)->data, (array)->capacity * (array)->stride, MEMORY_TAG_DYNAMIC_ARRAY); \
                    (array)->capacity = (new_cap);                                                                    \
                    (array)->data = new_data;                                                                         \
                }                                                                                                     \
                else                                                                                                  \
                {                                                                                                     \
                    LOG_FATAL("DYNAMIC_ARRAY_RESERVE: Fatal error");                                                  \
                    ASSERT(FALSE);                                                                                    \
                }                                                                                                     \
            }                                                                                                         \
        }                                                                                                             \
        else                                                                                                          \
        {                                                                                                             \
            LOG_FATAL("DYNAMIC_ARRAY_RESERVE: Fatal error");                                                          \
            ASSERT(FALSE);                                                                                            \
        }                                                                                                             \
    }                                                                                                                 \
    while (0)

#define DYNAMIC_ARRAY_RESIZE(array, count)                                                                            \
    do                                                                                                                \
    {                                                                                                                 \
        if ((array))                                                                                                  \
        {                                                                                                             \
            if ((count) > (array)->capacity)                                                                          \
            {                                                                                                         \
                void* new_data = memory_system_allocate((count) * (array)->stride, MEMORY_TAG_DYNAMIC_ARRAY);         \
                if (new_data)                                                                                         \
                {                                                                                                     \
                    memory_system_copy(new_data, (array)->data, (array)->size * (array)->stride);                     \
                    memory_system_free((array)->data, (array)->capacity * (array)->stride, MEMORY_TAG_DYNAMIC_ARRAY); \
                    (array)->capacity = (count);                                                                      \
                    (array)->size = (count);                                                                          \
                    (array)->data = new_data;                                                                         \
                }                                                                                                     \
                else                                                                                                  \
                {                                                                                                     \
                    LOG_FATAL("DYNAMIC_ARRAY_RESIZE: Fatal error");                                                   \
                    ASSERT(FALSE);                                                                                    \
                }                                                                                                     \
            }                                                                                                         \
        }                                                                                                             \
        else                                                                                                          \
        {                                                                                                             \
            LOG_FATAL("DYNAMIC_ARRAY_RESIZE: Fatal error");                                                           \
            ASSERT(FALSE);                                                                                            \
        }                                                                                                             \
    }                                                                                                                 \
    while (0)

#define DYNAMIC_ARRAY_PUSH(array, value)                                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        if ((array))                                                                                                               \
        {                                                                                                                          \
            if ((array)->size == (array)->capacity)                                                                                \
            {                                                                                                                      \
                DYNAMIC_ARRAY_RESERVE((array), (array)->capacity * 2);                                                             \
            }                                                                                                                      \
            memory_system_copy((char)(array)->data + (array)->size * (array)->stride, (void const*){ &(value) }, (array)->stride); \
            (array)->size++;                                                                                                       \
        }                                                                                                                          \
        else                                                                                                                       \
        {                                                                                                                          \
            LOG_FATAL("DYNAMIC_ARRAY_PUSH: Fatal error");                                                                          \
            ASSERT(FALSE);                                                                                                         \
        }                                                                                                                          \
    }                                                                                                                              \
    while (0)

#define DYNAMIC_ARRAY_POP(array, value)                                                                                   \
    do                                                                                                                    \
    {                                                                                                                     \
        if (array)                                                                                                        \
        {                                                                                                                 \
            if ((array)->size > 0)                                                                                        \
            {                                                                                                             \
                memory_system_copy(&(value), (char)(array)->data + (array)->size * (array)->stride - 1, (array)->stride); \
                (array)->size--;                                                                                          \
            }                                                                                                             \
            else                                                                                                          \
            {                                                                                                             \
                LOG_FATAL("DYNAMIC_ARRAY_POP: Fatal error");                                                              \
                ASSERT(FALSE);                                                                                            \
            }                                                                                                             \
        }                                                                                                                 \
        else                                                                                                              \
        {                                                                                                                 \
            LOG_FATAL("DYNAMIC_ARRAY_POP: Fatal error");                                                                  \
            ASSERT(FALSE);                                                                                                \
        }                                                                                                                 \
    }                                                                                                                     \
    while (0)

#include "dynamic_array.h"

#include "core/logger.h"
#include "systems/memory_system.h"

Dynamic_Array* dynamic_array_create(char const* type, u32 stride)
{
    Dynamic_Array* array = memory_system_allocate(sizeof(*array), MEMORY_TAG_CONTAINERS);
    strncpy(array->type, type, sizeof(array->type) - 1);
    array->capacity = 1;
    array->size = 0;
    array->stride = stride;
    array->data = memory_system_allocate(array->stride, MEMORY_TAG_CONTAINERS);
    return array;
}

void dynamic_array_destroy(Dynamic_Array* array)
{
    if (array->data)
    {
        memory_system_free(array->data, array->capacity * array->stride, MEMORY_TAG_CONTAINERS);
    }

    memory_system_free(array, sizeof(*array), MEMORY_TAG_CONTAINERS);
}

void dynamic_array_reserve(Dynamic_Array* array, u32 new_cap)
{
    if (new_cap > array->capacity)
    {
        void* new_data = memory_system_allocate(new_cap * array->stride, MEMORY_TAG_CONTAINERS);
        memory_system_copy(new_data, array->data, array->size * array->stride);
        memory_system_free(array->data, array->capacity * array->stride, MEMORY_TAG_CONTAINERS);
        array->capacity = new_cap;
        array->data = new_data;
    }
    else
    {
        LOG_WARNING("dynamic_array_reserve: Trying to shrink array. Not happening");
    }
}

void dynamic_array_resize(Dynamic_Array* array, u32 count)
{
    if (count > array->capacity)
    {
        void* new_data = memory_system_allocate(count * array->stride, MEMORY_TAG_CONTAINERS);
        memory_system_copy(new_data, array->data, array->size * array->stride);
        memory_system_free(array->data, array->capacity * array->stride, MEMORY_TAG_CONTAINERS);
        array->capacity = count;
        array->size = count;
        array->data = new_data;
    }
    else if (count > array->size)
    {
        array->size = count;
    }
    else
    {
        LOG_WARNING("dynamic_array_resize: Trying to shrink array. Not happening");
    }
}

void dynamic_array_push(Dynamic_Array* array, void const* value)
{
    if (array->size == array->capacity)
    {
        dynamic_array_reserve(array, array->capacity * 2);
    }

    memory_system_copy((char)array->data + array->size * array->stride, value, array->stride);
    array->size++;
}

void dynamic_array_pop(Dynamic_Array* array, void* value)
{
    if (array->size > 0)
    {
        memory_system_copy(value, (char)array->data + array->size * array->stride - 1, array->stride);
        array->size--;
    }
    else
    {
        LOG_WARNING("dynamic_array_pop: Trying to pop from empty array. Not happening");
        value = 0;
    }
}

void* dynamic_array_at(Dynamic_Array* array, u32 pos)
{
    if (pos < array->size)
    {
        return (char)array->data + pos * array->stride;
    }

    LOG_FATAL("dynamic_array_at: Range check failed");
    return 0;
}

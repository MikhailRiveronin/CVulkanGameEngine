#include "DynamicArray.h"
#include "Core/Logger.h"
#include "Core/Memory.h"

void* dynamicArrayCreate(u64 capacity, u64 stride)
{
    u64 headerSize = DYNAMIC_ARRAY_FIELD_COUNT * sizeof(u64);
    u64 size = headerSize + capacity * stride;
    u64* header = allocate(size, MEMORY_TAG_DYNAMIC_ARRAY);
    memoryZero(header, size);
    header[DYNAMIC_ARRAY_FIELD_CAPACITY] = capacity;
    header[DYNAMIC_ARRAY_FIELD_LENGTH] = 0;
    header[DYNAMIC_ARRAY_FIELD_STRIDE] = stride;
    return header + DYNAMIC_ARRAY_FIELD_COUNT;
}

void dynamicArrayDestroy(void const* array)
{
    u64* header = (u64*)array - DYNAMIC_ARRAY_FIELD_COUNT;
    u64 headerSize = DYNAMIC_ARRAY_FIELD_COUNT * sizeof(u64);
    u64 capacity = DYNAMIC_ARRAY_CAPACITY(array);
    u64 stride = DYNAMIC_ARRAY_STRIDE(array);
    u64 size = headerSize + capacity * stride;
    deallocate(header, size, MEMORY_TAG_DYNAMIC_ARRAY);
}

u64 dynamicArrayGetField(void const* array, DynamicArrayField field)
{
    u64* header = (u64*)array - DYNAMIC_ARRAY_FIELD_COUNT;
    return header[field];
}

void dynamicArraySetField(void const* array, DynamicArrayField field, u64 value)
{
    u64* header = (u64*)array - DYNAMIC_ARRAY_FIELD_COUNT;
    header[field] = value;
}

void* dynamicArrayResize(void const* array)
{
    u64 capacity = DYNAMIC_ARRAY_CAPACITY(array);
    u64 length = DYNAMIC_ARRAY_LENGTH(array);
    u64 stride = DYNAMIC_ARRAY_STRIDE(array);
    u64* temp = dynamicArrayCreate(DYNAMIC_ARRAY_RESIZE_FACTOR * capacity, stride);
    memoryCopy(temp, array, length);
    dynamicArrayDestroy(array);
    dynamicArraySetField(temp, DYNAMIC_ARRAY_FIELD_LENGTH, length);
    return temp;
}

void* dynamicArrayPush(void* array, void const* value)
{
    u64 capacity = DYNAMIC_ARRAY_CAPACITY(array);
    u64 length = DYNAMIC_ARRAY_LENGTH(array);
    u64 stride = DYNAMIC_ARRAY_STRIDE(array);
    if (length >= capacity) {
        array = dynamicArrayResize(array);
    }

    memoryCopy((char*)array + length * stride, value, stride);
    DYNAMIC_ARRAY_SET_LENGHT(array, length + 1);
    return array;
}

void dynamicArrayPop(void const* array, void* dest)
{
    u64 length = DYNAMIC_ARRAY_LENGTH(array);
    u64 stride = DYNAMIC_ARRAY_STRIDE(array);
    memoryCopy(dest, (char*)array + (length - 1) * stride, stride);
    DYNAMIC_ARRAY_SET_LENGHT(array, length - 1);
}

void* dynamicArrayInsert(void* array, u64 pos, void const* value)
{
    u64 capacity = DYNAMIC_ARRAY_CAPACITY(array);
    u64 length = DYNAMIC_ARRAY_LENGTH(array);
    u64 stride = DYNAMIC_ARRAY_STRIDE(array);
    if (pos >= length) {
        LOG_ERROR("pos outside the bounds of this array. Length: %llu, pos: %llu", length, pos);
        return array;
    }

    if (length >= capacity) {
        array = dynamicArrayResize(array);
    }

    if (pos != length - 1) {
        memoryCopy((char*)array + (pos + 1) * stride, (char*)array + pos * stride, (length - pos) * stride);
    }
    memoryCopy((char*)array + pos * stride, value, stride);
    DYNAMIC_ARRAY_SET_LENGHT(array, length + 1);
    return array;
}

void dynamicArrayErase(void const* array, u64 pos, void* dest)
{
    u64 length = DYNAMIC_ARRAY_LENGTH(array);
    u64 stride = DYNAMIC_ARRAY_STRIDE(array);
    if (pos >= length) {
        LOG_ERROR("pos outside the bounds of this array. Length: %llu, pos: %llu", length, pos);
        return;
    }

    memoryCopy(dest, (char*)array + pos * stride, stride);
    if (pos != length - 1) {
        memoryCopy((char*)array + pos * stride, (char*)array + (pos + 1) * stride, (length - pos - 1) * stride);
    }
    DYNAMIC_ARRAY_SET_LENGHT(array, length - 1);
}

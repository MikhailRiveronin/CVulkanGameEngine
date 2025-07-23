#pragma once

#include "Defines.h"

// Memory layout
// u64 capacity = number elements that can be held
// u64 length = number of elements currently contained
// u64 stride = size of each element in bytes
// void* elements
typedef enum DynamicArrayField {
    DYNAMIC_ARRAY_FIELD_CAPACITY,
    DYNAMIC_ARRAY_FIELD_LENGTH,
    DYNAMIC_ARRAY_FIELD_STRIDE,
    DYNAMIC_ARRAY_FIELD_COUNT
} DynamicArrayField;

void* dynamicArrayCreate(u64 capacity, u64 stride);
void dynamicArrayDestroy(void const* array);

u64 dynamicArrayGetField(void const* array, DynamicArrayField field);
void dynamicArraySetField(void const* array, DynamicArrayField field, u64 value);

void* dynamicArrayResize(void const* array);

void* dynamicArrayPush(void* array, void const* value);
void dynamicArrayPop(void const* array, void* dest);

void* dynamicArrayInsert(void* array, u64 pos, void const* value);
void dynamicArrayErase(void const* array, u64 pos, void* dest);

#define DYNAMIC_ARRAY_DEFAULT_CAPACITY 1
#define DYNAMIC_ARRAY_RESIZE_FACTOR 2

#define DYNAMIC_ARRAY_CREATE(type) dynamicArrayCreate(DYNAMIC_ARRAY_DEFAULT_CAPACITY, sizeof(type))
#define DYNAMIC_ARRAY_RESERVE(type, capacity) dynamicArrayCreate(capacity, sizeof(type))
#define DYNAMIC_ARRAY_DESTROY(array) dynamicArrayDestroy(array)

#define DYNAMIC_ARRAY_PUSH(array, value) array = dynamicArrayPush(array, &value)
#define DYNAMIC_ARRAY_POP(array, dest) dynamicArrayPop(array, dest)

#define DYNAMIC_ARRAY_INSERT(array, pos, value) array = dynamicArrayInsert(array, pos, &value)
#define DYNAMIC_ARRAY_ERASE(array, pos, dest) dynamicArrayErase(array, pos, dest)

#define DYNAMIC_ARRAY_CAPACITY(array) dynamicArrayGetField(array, DYNAMIC_ARRAY_FIELD_CAPACITY)
#define DYNAMIC_ARRAY_LENGTH(array) dynamicArrayGetField(array, DYNAMIC_ARRAY_FIELD_LENGTH)
#define DYNAMIC_ARRAY_STRIDE(array) dynamicArrayGetField(array, DYNAMIC_ARRAY_FIELD_STRIDE)

#define DYNAMIC_ARRAY_SET_LENGHT(array, value) dynamicArraySetField(array, DYNAMIC_ARRAY_FIELD_LENGTH, value)
#define DYNAMIC_ARRAY_CLEAR(array) DYNAMIC_ARRAY_SET_LENGHT(array, 0)

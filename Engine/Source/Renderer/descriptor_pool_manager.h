#pragma once

#include "defines.h"
#include "renderer_types.h"
#include "containers/dynamic_array.h"

typedef enum Pipeline_Size_Class
{
    PIPELINE_SIZE_CLASS_SHADOW_PASS,
    PIPELINE_SIZE_CLASS_MAIN_PASS
} Pipeline_Size_Class;

typedef struct Descriptor_Pool_Manager
{
    u32 max_set_count;
    u32 average_descriptor_counts[DESCRIPTOR_TYPE_ENUM_COUNT];
    Dynamic_Array* free_pools;

} Descriptor_Pool_Manager;

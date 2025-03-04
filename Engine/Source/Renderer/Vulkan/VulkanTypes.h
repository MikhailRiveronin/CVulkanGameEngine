#pragma once

#include "Defines.h"

#include "Core/Asserts.h"

#include <vulkan/vulkan.h>

#define CHECK_IF_VK_SUCCESS(expr, file, line, message)    \
    do                                                    \
    {                                                     \
        if (expr != VK_SUCCESS)                           \
        {                                                 \
            LOG_ERROR("%s(%u): %s", file, line, message); \
            ASSERT(FALSE);                                \
        }                                                 \
    }                                                     \
    while (0)

typedef struct VulkanContext
{
    void* allocator;
    VkInstance instance;
} VulkanContext;

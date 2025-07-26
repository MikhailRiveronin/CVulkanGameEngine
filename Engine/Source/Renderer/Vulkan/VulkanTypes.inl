#pragma once

#include "Defines.h"
#include "Core/Asserts.h"
#include "Containers/DArray.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)                                                    \
    do {                                                                  \
        if (expr != VK_SUCCESS) {                                         \
            LOG_ERROR("%s(%u): %s", __FILE__, __LINE__, "Vulkan failed"); \
            ASSERT(FALSE);                                                \
        }                                                                 \
    }                                                                     \
    while (0)

typedef struct VulkanSwapchainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    DARRAY(VkSurfaceFormatKHR) formats;
    DARRAY(VkPresentModeKHR) modes;
} VulkanSwapchainSupportInfo;

typedef struct VulkanDevice {
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VulkanSwapchainSupportInfo swapchainSupport;

    struct {
        struct {
            VkQueue handle;
            u32 index;
        } graphics;

        struct {
            VkQueue handle;
            u32 index;
        } compute;

        struct {
            VkQueue handle;
            u32 index;
        } transfer;

        struct {
            VkQueue handle;
            u32 index;
        } present;
    } queues;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
} VulkanDevice;

typedef struct VulkanContext {
    VkInstance instance;
    VkAllocationCallbacks* allocator;
#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugUtilsMessenger;
#endif
    VkSurfaceKHR surface;
    VulkanDevice device;
} VulkanContext;

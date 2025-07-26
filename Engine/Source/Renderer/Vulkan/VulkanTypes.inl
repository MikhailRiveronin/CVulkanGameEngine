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

    VkFormat depthFormat;
} VulkanDevice;

typedef struct VulkanImage {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} VulkanImage;

typedef struct VulkanSwapchain {
    VkSwapchainKHR handle;
    VkSurfaceFormatKHR surfaceFormat;
    u8 framesInFlight;
    DARRAY(VkImage) images;
    DARRAY(VkImageView) imageViews;

    VulkanImage depthBuffer;
} VulkanSwapchain;

typedef struct VulkanContext {
    VkInstance instance;
    VkAllocationCallbacks* allocator;
#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugUtilsMessenger;
#endif
    VkSurfaceKHR surface;
    VulkanDevice device;

    struct {
        u32 width;
        u32 height;
    } framebuffer;

    VulkanSwapchain swapchain;
    u32 imageIndex;
    u32 currentFrame;
    b8 recreateSwapchain;

    i32 (* findMemoryType)(u32 memoryTypeBits, VkMemoryPropertyFlags properties);
} VulkanContext;

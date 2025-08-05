#pragma once

#include "defines.h"
#include "Core/Asserts.h"
#include "Containers/darray.h"

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

typedef struct vulkan_device {
    VkPhysicalDevice physical_device;
    VkDevice handle;
    VulkanSwapchainSupportInfo swapchain_support;

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

    VkCommandPool graphicsCommandPool;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    VkFormat depthFormat;
} vulkan_device;

typedef struct VulkanImage {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} VulkanImage;

typedef enum VulkanRenderPassState {
    RENDERPASS_READY,
    RENDERPASS_RECORDING,
    RENDERPASS_IN_RENDER_PASS,
    RENDERPASS_RECORDING_ENDED,
    RENDERPASS_SUBMITTED,
    RENDERPASS_NOT_ALLOCATED
} VulkanRenderPassState;

typedef struct vulkan_renderpass {
    VkRenderPass handle;
    f32 x, y, width, height;
    f32 r, g, b, a;

    f32 depth;
    u32 stencil;

    VulkanRenderPassState state;
} vulkan_renderpass;

typedef struct VulkanFramebuffer {
    VkFramebuffer handle;
    u32 attachmentCount;
    VkImageView* attachments;
    vulkan_renderpass* renderpass;
} VulkanFramebuffer;

typedef struct VulkanSwapchain {
    VkSwapchainKHR handle;
    VkSurfaceFormatKHR surfaceFormat;
    u8 frames_in_flight;
    DARRAY(VkImage) images;
    DARRAY(VkImageView) imageViews;
    DARRAY(VulkanFramebuffer) framebuffers;

    VulkanImage depthBuffer;
} VulkanSwapchain;

typedef enum VulkanCommandBufferState {
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} VulkanCommandBufferState;

typedef struct vulkan_command_buffer {
    VkCommandBuffer handle;
    VulkanCommandBufferState state;
} vulkan_command_buffer;

typedef struct VulkanFence {
    VkFence handle;
    b8 signaled;
} VulkanFence;

typedef struct vulkan_context {
    i16 framebuffer_width;
    i16 framebuffer_height;
    u64 framebuffer_generation;
    u64 framebuffer_last_generation;

    VulkanSwapchain swapchain;
    b8 recreating_swapchain;
    u32 image_index;

    // Sync objects
    DARRAY(VkSemaphore) image_available_semaphors;
    DARRAY(VkSemaphore) render_complete_semaphors;
    DARRAY(VulkanFence) fences_in_flight;
    DARRAY(VulkanFence*) imagesInFlight;
    u32 current_frame;

    DARRAY(vulkan_command_buffer) command_buffers;
    vulkan_renderpass main_renderpass;




    VkInstance instance;
    VkAllocationCallbacks* allocator;
#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugUtilsMessenger;
#endif
    VkSurfaceKHR surface;
    vulkan_device device;









    i32 (* findMemoryType)(u32 memoryTypeBits, VkMemoryPropertyFlags properties);
} vulkan_context;

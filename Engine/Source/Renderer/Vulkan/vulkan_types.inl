#pragma once

#include "defines.h"
#include "core/asserts.h"
#include "containers/darray.h"
#include "renderer/renderer_types.inl"
#include <vulkan/vulkan.h>

#define VK_CHECK(expr)                                                    \
    do {                                                                  \
        if (expr != VK_SUCCESS) {                                         \
            LOG_ERROR("%s(%u): %s", __FILE__, __LINE__, "Vulkan failed"); \
            ASSERT(FALSE);                                                \
        }                                                                 \
    }                                                                     \
    while (0)

typedef struct vulkan_buffer {
    VkBuffer handle;
    VkDeviceMemory memory;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags memory_properties;
    i32 memory_index;
    void* mapped;
    u32 alignment;
    b8 locked;
} vulkan_buffer;

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

    VkCommandPool graphics_command_pool;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    VkFormat depthFormat;
} vulkan_device;

typedef struct vulkan_image {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} vulkan_image;

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

    vulkan_image depthBuffer;
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

typedef struct vulkan_shader_stage {
    VkShaderModuleCreateInfo create_info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
} vulkan_shader_stage;

typedef struct vulkan_pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;
} vulkan_pipeline;

#define MATERIAL_SHADER_STAGE_COUNT 2

#define VULKAN_MAX_MATERIAL_COUNT 1024

typedef struct vulkan_descriptor_state {
    u32 generations[3];
} vulkan_descriptor_state;

#define VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_MATERIAL_SHADER_SAMPLER_COUNT 2

typedef struct vulkan_material_shader_instance_state {
    VkDescriptorSet descriptor_sets[3];
    vulkan_descriptor_state descriptor_states[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
} vulkan_material_shader_instance_state;

typedef struct vulkan_material_shader {
    vulkan_shader_stage stages[MATERIAL_SHADER_STAGE_COUNT];
    vulkan_pipeline pipeline;

    VkDescriptorPool global_descriptor_pool;
    VkDescriptorSetLayout global_descriptor_set_layout;
    vulkan_buffer global_ubo;

    VkDescriptorPool object_descriptor_pool;
    VkDescriptorSetLayout object_descriptor_set_layout;
    vulkan_buffer object_ubo;
    u32 object_ubo_index;

    texture_use sampler_uses[VULKAN_MATERIAL_SHADER_SAMPLER_COUNT];

    vulkan_material_shader_instance_state instance_states[VULKAN_MAX_MATERIAL_COUNT];

    // Triple-buffering
    VkDescriptorSet global_descriptor_sets[3];
    VkDescriptorSet object_descriptor_sets[3];

    global_uniform_data global_data;
    object_uniform_data object_data;
} vulkan_material_shader;

typedef struct vulkan_context {
    f32 frame_delta_time;

    i16 framebuffer_width;
    i16 framebuffer_height;
    u64 framebuffer_generation;
    u64 framebuffer_last_generation;

    vulkan_buffer object_vertex_buffer;
    vulkan_buffer object_index_buffer;

    u64 geometry_vertex_offset;
    u64 geometry_index_offset;

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

    vulkan_material_shader material_shader;

    i32 (* findMemoryType)(u32 memoryTypeBits, VkMemoryPropertyFlags properties);
} vulkan_context;

typedef struct vulkan_texture_data {
    vulkan_image image;
    VkSampler sampler;
} vulkan_texture_data;

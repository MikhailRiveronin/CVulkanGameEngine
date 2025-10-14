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
    vec4 render_area;
    vec4 clear_colour;

    f32 depth;
    u32 stencil;

    u8 clear_flags;
    b8 has_prev_pass;
    b8 has_next_pass;

    VulkanRenderPassState state;
} vulkan_renderpass;

typedef struct VulkanSwapchain {
    VkSwapchainKHR handle;
    VkSurfaceFormatKHR surfaceFormat;
    u8 frames_in_flight;
    DARRAY(VkImage) images;
    DARRAY(VkImageView) image_views;
    VkFramebuffer framebuffers[3];

    vulkan_image depth_buffer;
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


typedef struct vulkan_descriptor_state {
    u32 generations[3];
    u32 ids[3];
} vulkan_descriptor_state;

#define VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT 2

/**
 * @brief Internal buffer data for geometry.
 */
typedef struct vulkan_geometry_data {
    u32 id;
    u32 generation;

    u32 vertex_count;
    u32 vertex_element_size;
    u32 vertex_buffer_offset;

    u32 index_count;
    u32 index_element_size;
    u32 index_buffer_offset;
} vulkan_geometry_data;

typedef struct vulkan_material_shader_instance_state {
    VkDescriptorSet descriptor_sets[3];
    vulkan_descriptor_state descriptor_states[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
} vulkan_material_shader_instance_state;

typedef struct vulkan_material_shader_global_data {
    mat4 view;
    mat4 proj;
    mat4 reserved0;
    mat4 reserved1;
} vulkan_material_shader_global_data;

#define MATERIAL_SHADER_STAGE_COUNT 2
#define VULKAN_MATERIAL_SHADER_SAMPLER_COUNT 2

#define PREDEFINED_MATERIAL_SHADER_NAME "predefined.material_shader"

typedef struct vulkan_shader_stage {
    struct {
        VkShaderModuleCreateInfo create_info;
        VkShaderModule handle;
    } module;

    VkPipelineShaderStageCreateInfo create_info;
} vulkan_shader_stage;

typedef struct vulkan_pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;
} vulkan_pipeline;

#define VULKAN_MAX_MATERIAL_COUNT 1024
#define VULKAN_MAX_GEOMETRY_COUNT 4096

typedef struct vulkan_material_shader {
    vulkan_shader_stage stages[MATERIAL_SHADER_STAGE_COUNT];

    VkDescriptorPool global_descriptor_pool;
    VkDescriptorPool object_descriptor_pool;

    VkDescriptorSetLayout global_descriptor_set_layout;
    VkDescriptorSetLayout object_descriptor_set_layout;

    VkDescriptorSet global_descriptor_sets[3];
    VkDescriptorSet object_descriptor_sets[3];

    vulkan_material_shader_global_data global_data;

    vulkan_buffer global_ubo;
    vulkan_buffer object_ubo;

    u32 object_ubo_index;

    texture_use sampler_uses[VULKAN_MATERIAL_SHADER_SAMPLER_COUNT];

    vulkan_material_shader_instance_state instance_states[VULKAN_MAX_MATERIAL_COUNT];

    vulkan_pipeline pipeline;

    // TODO: check if needed
    // material_uniform_data object_data;
} vulkan_material_shader;

#define UI_SHADER_STAGE_COUNT 2
#define VULKAN_UI_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_UI_SHADER_SAMPLER_COUNT 1

// Max number of ui control instances
// TODO: make configurable
#define VULKAN_MAX_UI_COUNT 1024

typedef struct vulkan_ui_shader_instance_state {
    // Per frame
    VkDescriptorSet descriptor_sets[3];

    // Per descriptor
    vulkan_descriptor_state descriptor_states[VULKAN_UI_SHADER_DESCRIPTOR_COUNT];
} vulkan_ui_shader_instance_state;

/**
 * @brief Vulkan-specific uniform buffer object for the ui shader. 
 */
typedef struct vulkan_ui_shader_global_ubo {
    mat4 projection;   // 64 bytes
    mat4 view;         // 64 bytes
    mat4 m_reserved0;  // 64 bytes, reserved for future use
    mat4 m_reserved1;  // 64 bytes, reserved for future use
} vulkan_ui_shader_global_ubo;

/**
 * @brief Vulkan-specific ui material instance uniform buffer object for the ui shader. 
 */
typedef struct vulkan_ui_shader_instance_ubo {
    vec4 diffuse_color;  // 16 bytes
    vec4 v_reserved0;    // 16 bytes, reserved for future use
    vec4 v_reserved1;    // 16 bytes, reserved for future use
    vec4 v_reserved2;    // 16 bytes, reserved for future use
} vulkan_ui_shader_instance_ubo;

typedef struct vulkan_ui_shader {
    // vertex, fragment
    vulkan_shader_stage stages[UI_SHADER_STAGE_COUNT];

    VkDescriptorPool global_descriptor_pool;
    VkDescriptorSetLayout global_descriptor_set_layout;

    // One descriptor set per frame - max 3 for triple-buffering.
    VkDescriptorSet global_descriptor_sets[3];

    // Global uniform object.
    vulkan_material_shader_global_data global_ubo;

    // Global uniform buffer.
    vulkan_buffer global_uniform_buffer;

    VkDescriptorPool object_descriptor_pool;
    VkDescriptorSetLayout object_descriptor_set_layout;
    // Object uniform buffers.
    vulkan_buffer object_uniform_buffer;
    // TODO: manage a free list of some kind here instead.
    u32 object_uniform_buffer_index;

    texture_use sampler_uses[VULKAN_UI_SHADER_SAMPLER_COUNT];

    // TODO: make dynamic
    vulkan_ui_shader_instance_state instance_states[VULKAN_MAX_UI_COUNT];

    vulkan_pipeline pipeline;

} vulkan_ui_shader;

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
    u32 current_image;

    // Sync objects
    DARRAY(VkSemaphore) image_available_semaphors;
    DARRAY(VkSemaphore) render_complete_semaphors;

    VkFence fences_in_flight[2];
    VkFence* images_in_flight[3];

    u32 current_frame;

    DARRAY(vulkan_command_buffer) command_buffers;
    vulkan_renderpass main_renderpass;
    vulkan_renderpass ui_renderpass;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugUtilsMessenger;
#endif
    VkSurfaceKHR surface;
    vulkan_device device;

    vulkan_material_shader material_shader;
    vulkan_ui_shader ui_shader;

    // Framebuffers used for world rendering, one per frame
    VkFramebuffer world_framebuffers[3];

    i32 (* findMemoryType)(u32 memoryTypeBits, VkMemoryPropertyFlags properties);

    vulkan_geometry_data geometries[VULKAN_MAX_GEOMETRY_COUNT];
} vulkan_context;

typedef struct vulkan_texture_data {
    vulkan_image image;
    VkSampler sampler;
} vulkan_texture_data;

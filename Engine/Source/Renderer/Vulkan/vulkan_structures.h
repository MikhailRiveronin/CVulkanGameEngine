#pragma once

#include "defines.h"
#include "containers/darray.h"
#include "containers/freelist.h"
#include "core/asserts.h"
#include "renderer/renderer_types.h"

#include <vulkan/vulkan.h>





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














/**
 * @brief Internal buffer data for geometry.
 */
typedef struct vulkan_geometry_buffer_data
{
    u32 id;
    u32 generation;
    u32 vertex_count;
    u32 vertex_size_in_bytes;
    u64 vertex_buffer_offset;
    u32 index_count;
    u32 index_size_in_bytes;
    u64 index_buffer_offset;
} vulkan_geometry_buffer_data;






// #define PREDEFINED_MATERIAL_SHADER_NAME "predefined.material_shader"







// // TODO: Make configurable.
// 







// // Max number of ui control instances
// // TODO: make configurable




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







typedef struct vulkan_buffer
{
    VkBuffer handle;
    VkDeviceMemory device_memory;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags memory_property;
    i32 memory_index;
    u32 alignment;
    b8 locked;

    struct
    {
        freelist tracker;
        u64 required_memory;
        void* block;
    } suballocation;
    VkAllocationCallbacks allocator;
} vulkan_buffer;





#define VULKAN_MAX_MATERIAL_COUNT 1024
#define VULKAN_MAX_GEOMETRY_COUNT 4096



typedef struct vulkan_swapchain
{
    VkSwapchainKHR handle;
    VkSurfaceFormatKHR surfaceFormat;
    u8 frames_in_flight;
    DARRAY(VkImage) images;
    DARRAY(VkImageView) image_views;
    VkFramebuffer framebuffers[3];

    vulkan_image depth_buffer;
} vulkan_swapchain;



#define MATERIAL_SHADER_STAGE_COUNT 2
#define VULKAN_MATERIAL_SHADER_SAMPLER_COUNT 2

typedef struct vulkan_shader_stage
{
    struct {
        VkShaderModuleCreateInfo create_info;
        VkShaderModule handle;
    } module;
    VkPipelineShaderStageCreateInfo create_info;
} vulkan_shader_stage;

typedef struct vulkan_pipeline
{
    VkPipeline handle;
    VkPipelineLayout layout;
} vulkan_pipeline;

typedef struct vulkan_material_shader_global_ubo_data
{
    mat4 view;
    mat4 proj;
    mat4 reserved_mat4_0;
    mat4 reserved_mat4_1;
} vulkan_material_shader_global_ubo_data;

#define VULKAN_MAX_FRAME_COUNT 3

typedef struct vulkan_descriptor_state
{
    u32 generations[VULKAN_MAX_FRAME_COUNT];
    u32 ids[VULKAN_MAX_FRAME_COUNT];
} vulkan_descriptor_state;

#define VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT 2

typedef struct vulkan_material_shader_object_state
{
    VkDescriptorSet descriptor_sets[VULKAN_MAX_FRAME_COUNT];
    vulkan_descriptor_state descriptor_states[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
} vulkan_material_shader_object_state;

typedef struct vulkan_material_shader
{
    vulkan_shader_stage stages[MATERIAL_SHADER_STAGE_COUNT];

    VkDescriptorPool global_descriptor_pool;
    VkDescriptorPool object_descriptor_pool;

    VkDescriptorSetLayout global_descriptor_set_layout;
    VkDescriptorSetLayout object_descriptor_set_layout;

    VkDescriptorSet global_descriptor_sets[3];
    VkDescriptorSet object_descriptor_sets[3];

    vulkan_material_shader_global_ubo_data global_data;

    vulkan_buffer global_ubo;
    vulkan_buffer object_ubo;

    u32 object_ubo_index;

    Texture_Use sampler_uses[VULKAN_MATERIAL_SHADER_SAMPLER_COUNT];

    vulkan_material_shader_object_state instance_states[VULKAN_MAX_MATERIAL_COUNT];

    vulkan_pipeline pipeline;

    // TODO: check if needed
    // material_uniform_data object_data;
} vulkan_material_shader;

#define UI_SHADER_STAGE_COUNT 2
#define VULKAN_UI_SHADER_SAMPLER_COUNT 1
#define VULKAN_UI_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_MAX_UI_COUNT 1024

typedef struct vulkan_ui_shader_instance_state {
    // Per frame
    VkDescriptorSet descriptor_sets[3];

    // Per descriptor
    vulkan_descriptor_state descriptor_states[VULKAN_UI_SHADER_DESCRIPTOR_COUNT];
} vulkan_ui_shader_instance_state;

typedef struct vulkan_ui_shader
{
    // vertex, fragment
    vulkan_shader_stage stages[UI_SHADER_STAGE_COUNT];

    VkDescriptorPool global_descriptor_pool;
    VkDescriptorSetLayout global_descriptor_set_layout;

    // One descriptor set per frame - max 3 for triple-buffering.
    VkDescriptorSet global_descriptor_sets[3];

    // Global uniform object.
    vulkan_material_shader_global_ubo_data global_ubo;

    // Global uniform buffer.
    vulkan_buffer global_uniform_buffer;

    VkDescriptorPool object_descriptor_pool;
    VkDescriptorSetLayout object_descriptor_set_layout;
    // Object uniform buffers.
    vulkan_buffer object_uniform_buffer;
    // TODO: manage a free list of some kind here instead.
    u32 object_uniform_buffer_index;

    Texture_Use sampler_uses[VULKAN_UI_SHADER_SAMPLER_COUNT];

    // TODO: make dynamic
    vulkan_ui_shader_instance_state instance_states[VULKAN_MAX_UI_COUNT];

    vulkan_pipeline pipeline;

} vulkan_ui_shader;

typedef enum VulkanCommandBufferState
{
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} VulkanCommandBufferState;

typedef struct vulkan_command_buffer
{
    VkCommandBuffer handle;
    VulkanCommandBufferState state;
} vulkan_command_buffer;











typedef struct vulkan_texture_resource
{
    vulkan_image image;
    VkSampler sampler;
} vulkan_texture_resource;



typedef struct vulkan_material_shader_object_ubo_data
{
    vec4 diffuse_color;
    vec4 reserved_vec4_0;
    vec4 reserved_vec4_1;
    vec4 reserved_vec4_2;
    mat4 reserved_mat4_0;
    mat4 reserved_mat4_1;
    mat4 reserved_mat4_2;
} vulkan_material_shader_object_ubo_data;

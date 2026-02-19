#pragma once

#include "defines.h"
#include "containers/dynamic_array.h"
#include "containers/hash_table.h"
#include "resources/resource_types.h"

// #include "third_party/cglm/cglm.h"

#define VULKAN_CHECK_RESULT(expr)                                                    \
    do                                                                    \
    {                                                                     \
        if ((expr) != VK_SUCCESS)                                           \
        {                                                                 \
            LOG_FATAL("%s(%u): %s", __FILE__, __LINE__, "Vulkan failed"); \
            return false;                                                \
        }                                                                 \
    }                                                                     \
    while (0)

#define SPV_REFLECT_CHECK_RESULT(expr)                                                    \
    do                                                                    \
    {                                                                     \
        if ((expr) != SPV_REFLECT_RESULT_SUCCESS)                                           \
        {                                                                 \
            LOG_FATAL("%s(%u): %s", __FILE__, __LINE__, "SPIRV-Reflect failed"); \
            return false;                                                \
        }                                                                 \
    }                                                                     \
    while (0)



// typedef struct vulkan_shader_stage_config {
//     /** @brief The shader stage bit flag. */
//     VkShaderStageFlagBits stage;
//     /** @brief The shader file name. */
//     char file_name[255];

// } vulkan_shader_stage_config;

/**
 * @brief Pipeline shader stage-related data.
 */
typedef struct Pipeline_Shader_Stage
{
    // VkShaderModuleCreateInfo shader_module_create_info;
    // VkShaderModule shader_module;
    VkPipelineShaderStageCreateInfo create_info;
} Pipeline_Shader_Stage;


/**
 * @brief The current state of a shader.
 */
typedef enum Shader_State
{
    SHADER_STATE_UNINITIALIZED,
    SHADER_STATE_INITIALIZED
} Shader_State;

/**
 * @brief A shader vertex attribute.
 */
typedef struct Vertex_Attribute
{
    char name[32];
    VkFormat type;
    u32 size;
} Vertex_Attribute;

typedef enum Descriptor_Set_Scope
{
    DESCRIPTOR_SET_SCOPE_PER_FRAME,
    DESCRIPTOR_SET_SCOPE_PER_MATERIAL,
    DESCRIPTOR_SET_SCOPE_PER_OBJECT
} Descriptor_Set_Scope;

/**
 * @brief An entry in the uniform array.
 */
typedef struct Uniform_Buffer
{
    u64 offset;
    u16 location;
    u16 index;
    u16 size;
    u8 set_index;
    Descriptor_Set_Scope scope;
    VkFormat type;
} Uniform_Buffer;

// #define MAX_SHADER_STAGE_COUNT 8

typedef enum Pipeline_Shader_Stage
{
    PIPELINE_SHADER_STAGE_VERTEX,
    PIPELINE_SHADER_STAGE_FRAGMENT,
    PIPELINE_SHADER_STAGE_ENUM_COUNT
} Pipeline_Shader_Stage;

/**
 * @brief Represents a shader.
 */
typedef struct Shader
{
    Dynamic_Array* spv_paths;

    u32 id;
    Shader_Config_Resource* config;
    Shader_State state;

    // Simplifying assumption: All descriptors are organized into 3 sets
    VkDescriptorSetLayout descriptor_set_layouts[3];

    struct
    {
        VkPipeline handle;
        VkPipelineLayout pipeline_layout;
    } pipeline;

    struct
    {
        /** @brief The internal renderpass handle. */
        VkRenderPass handle;
        /** @brief The current render area of the renderpass. */
        vec4 render_area;
        /** @brief The clear colour used for this renderpass. */
        vec4 clear_colour;

        /** @brief The depth clear value. */
        f32 depth;
        /** @brief The stencil clear value. */
        u32 stencil;

        /** @brief The clear flags for this renderpass. */
        u8 clear_flags;

        /** @brief Indicates if there is a previous renderpass. */
        b8 has_prev_pass;
        /** @brief Indicates if there is a next renderpass. */
        b8 has_next_pass;

        /** @brief Indicates renderpass state. */
        vulkan_render_pass_state state;
    } renderpass;










    // Dynamic_Array* stages;
    // Dynamic_Array* stage_set_layouts[PIPELINE_SHADER_STAGE_ENUM_COUNT];




    Dynamic_Array* attributes;
    Dynamic_Array* global_textures;
    Dynamic_Array* uniforms;

    String_Map uniform_indices;






    /** @brief The block of memory mapped to the uniform buffer. */
    void* mapped_uniform_buffer_block;





    /** @brief The descriptor pool used for this shader. */
    VkDescriptorPool descriptor_pool;

    
    /** @brief Global descriptor sets, one per frame. */
    VkDescriptorSet global_descriptor_sets[3];
    /** @brief The uniform buffer used by this shader. */
    vulkan_buffer uniform_buffer;








    Pipeline_Shader_Stage pipeline_stages[MAX_SHADER_STAGE_COUNT];




    /** @brief The instance states for all instances. @todo TODO: make dynamic */
    u32 instance_count;
    vulkan_shader_instance_state instance_states[VULKAN_MAX_MATERIAL_COUNT];





} Shader;

typedef struct Texture
{
    char name[32];
    u32 id;
    u32 generation;
    u32 width;
    u32 height;
    u8 channel_count;
    b8 has_transparency;
    void* internal;
} Texture;



/**
 * @brief The overall context for the renderer.
 */
typedef struct Renderer_Context
{
    /** @brief The time in seconds since the last frame. */
    f32 frame_delta_time;

    /** @brief The framebuffer's current width. */
    u32 framebuffer_width;

    /** @brief The framebuffer's current height. */
    u32 framebuffer_height;

    /** @brief Current generation of framebuffer size. If it does not match framebuffer_size_last_generation, a new one should be generated. */
    u64 framebuffer_size_generation;

    /** @brief The generation of the framebuffer when it was last created. Set to framebuffer_size_generation when updated. */
    u64 framebuffer_size_last_generation;

    /** @brief The handle to the internal Vulkan instance. */
    VkInstance instance;
    /** @brief The internal Vulkan allocator. */
    VkAllocationCallbacks* allocator;
    /** @brief The internal Vulkan surface for the window to be drawn to. */
    VkSurfaceKHR surface;

#if defined(_DEBUG)
    /** @brief The debug messenger, if active.. */
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    /** @brief The Vulkan device. */
    struct
    {
        VkDevice handle;
        VkPhysicalDevice physical_device;
        VulkanSwapchainSupportInfo swapchain_support;

        struct
        {
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
    } device;

    /** @brief The swapchain. */
    vulkan_swapchain swapchain;

    /** @brief The main world renderpass. */
    Renderpass main_renderpass;

    /** @brief The UI renderpass. */
    Renderpass ui_renderpass;

    /** @brief The object vertex buffer, used to hold geometry vertices. */
    vulkan_buffer object_vertex_buffer;
    /** @brief The object index buffer, used to hold geometry indices. */
    vulkan_buffer object_index_buffer;

    /** @brief The graphics command buffers, one per frame. @note: darray */
    vulkan_command_buffer* graphics_command_buffers;

    /** @brief The semaphores used to indicate image availability, one per frame. @note: darray */
    VkSemaphore* image_available_semaphores;

    /** @brief The semaphores used to indicate queue availability, one per frame. @note: darray */
    VkSemaphore* queue_complete_semaphores;

    /** @brief The current number of in-flight fences. */
    u32 in_flight_fence_count;
    /** @brief The in-flight fences, used to indicate to the application when a frame is busy/ready. */
    VkFence in_flight_fences[2];

    /** @brief Holds pointers to fences which exist and are owned elsewhere, one per frame. */
    VkFence images_in_flight[3];

    /** @brief The current image index. */
    u32 image_index;

    /** @brief The current frame. */
    u32 current_frame;

    /** @brief Indicates if the swapchain is currently being recreated. */
    b8 recreating_swapchain;

    /** @brief The A collection of loaded geometries. @todo TODO: make dynamic */
    vulkan_geometry_data geometries[VULKAN_MAX_GEOMETRY_COUNT];

    /** @brief Framebuffers used for world rendering. @note One per frame. */
    VkFramebuffer world_framebuffers[3];

    /**
     * @brief A function pointer to find a memory index of the given type and with the given properties.
     * @param type_filter The types of memory to search for.
     * @param property_flags The required properties which must be present.
     * @returns The index of the found memory type. Returns -1 if not found.
     */
    i32 (*find_memory_index)(u32 type_filter, u32 property_flags);

} Renderer_Context;

typedef enum Descriptor_Type
{
    DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    DESCRIPTOR_TYPE_ENUM_COUNT
} Descriptor_Type;
















typedef enum renderer_backend_type {
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} renderer_backend_type;

typedef enum builtin_renderpass {
    BUILTIN_RENDERPASS_WORLD = 0x01,
    BUILTIN_RENDERPASS_UI = 0x02
} builtin_renderpass;



typedef struct material_uniform_data {
    vec4 diffuse_color;
    vec4 reserved0;
    vec4 reserved1;
    vec4 reserved2;
} material_uniform_data;

typedef struct geometry_render_data {
    mat4 world;
    Geometry* geometry;
} geometry_render_data;

struct platform_state;
struct Texture;

typedef struct renderer_backend {
    struct platform_state* plat_state;
    u64 frameCount;

    b8 (* startup)(struct renderer_backend* backend, char const* appName);
    void (* shutdown)(struct renderer_backend* backend);

    b8 (* begin_frame)(struct renderer_backend* backend, f64 delta_time);
    b8 (* end_frame)(struct renderer_backend* backend, f64 delta_time);

    void (* resize)(struct renderer_backend* backend, i16 width, i16 height);
    
    void (* update_global_state)(mat4 view, mat4 proj, vec3 view_pos, vec4 ambient_color, i32 mode);
    void (* update_global_ui_state)(mat4 proj, mat4 view, i32 mode);
    void (* draw_geometry)(geometry_render_data render_data);

    void (* create_texture)(u8 const* pixels, Texture* texture);
    void (* destroy_texture)(Texture* texture);

    b8 (*begin_renderpass)(struct renderer_backend* backend, u8 renderpass_id);
    b8 (*end_renderpass)(struct renderer_backend* backend, u8 renderpass_id);

    b8 (* create_material)(Material* material);
    void (* destroy_material)(Material* material);

    b8 (* create_geometry)(
        Geometry* geometry,
        u32 vertex_size_in_bytes,
        u32 vertex_count,
        void const* vertices,
        u32 index_size_in_bytes,
        u32 index_count,
        u32 const* indices);
    void (* destroy_geometry)(Geometry* geometry);
} renderer_backend;

typedef struct render_packet {
    f64 delta_time;

    u32 geometry_count;
    geometry_render_data* render_data;

    u32 ui_geometry_count;
    geometry_render_data* ui_render_data;
} render_packet;




#pragma once

#include "defines.h"
// #include "third_party/cglm/cglm.h"
// #include "resources/resources.h"

typedef struct Renderpass
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
} Renderpass;

typedef struct vulkan_shader_stage_config {
    /** @brief The shader stage bit flag. */
    VkShaderStageFlagBits stage;
    /** @brief The shader file name. */
    char file_name[255];

} vulkan_shader_stage_config;



/**
 * @brief Represents a shader.
 */
typedef struct Shader
{
    /** @brief The block of memory mapped to the uniform buffer. */
    void* mapped_uniform_buffer_block;

    /** @brief The shader identifier. */
    u32 id;

    Shader_Config config;

    /** @brief A pointer to the renderpass to be used with this shader. */
    Renderpass* renderpass;

    /** @brief An array of stages (such as vertex and fragment) for this shader. Count is located in config.*/
    vulkan_shader_stage stages[VULKAN_SHADER_MAX_STAGES];

    /** @brief The descriptor pool used for this shader. */
    VkDescriptorPool descriptor_pool;

    /** @brief Descriptor set layouts, max of 2. Index 0=global, 1=instance. */
    VkDescriptorSetLayout descriptor_set_layouts[2];
    /** @brief Global descriptor sets, one per frame. */
    VkDescriptorSet global_descriptor_sets[3];
    /** @brief The uniform buffer used by this shader. */
    vulkan_buffer uniform_buffer;

    /** @brief The pipeline associated with this shader. */
    vulkan_pipeline pipeline;

    /** @brief The instance states for all instances. @todo TODO: make dynamic */
    u32 instance_count;
    vulkan_shader_instance_state instance_states[VULKAN_MAX_MATERIAL_COUNT];

} Shader;

/**
 * @brief The overall context for the renderer.
 */
typedef struct Context
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
    vulkan_device device;

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

} Context;
















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




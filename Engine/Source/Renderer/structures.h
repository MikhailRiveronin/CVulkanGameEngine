#include "defines.h"
#include "third_party/cglm/struct.h"

typedef enum Renderpass_Clear_Flags
{
    RENDERPASS_CLEAR_FLAG_NONE = 0x0,
    RENDERPASS_CLEAR_FLAG_COLOR = 0x1,
    RENDERPASS_CLEAR_FLAG_DEPTH = 0x2,
    RENDERPASS_CLEAR_FLAG_STENCIL = 0x4
} Renderpass_Clear_Flags;

typedef enum Renderpass_State
{
    RENDERPASS_STATE_READY,
    RENDERPASS_STATE_RECORDING,
    RENDERPASS_STATE_IN_RENDER_PASS,
    RENDERPASS_STATE_RECORDING_ENDED,
    RENDERPASS_STATE_SUBMITTED,
    RENDERPASS_STATE_NOT_ALLOCATED
} Renderpass_State;

typedef struct Renderpass
{
    struct
    {
        vec4s color;
        f32 depth;
        u32 stencil;
        Renderpass_Clear_Flags clear_flags;
    } clear_values;

    VkRenderPass handle;
    ivec4s render_area;
    b8 has_prev;
    b8 has_next;
    Renderpass_State state;
} Renderpass;

typedef struct Context
{
    Renderpass main_renderpass;
    Renderpass ui_renderpass;















    char const* app_name;
    u32 framebuffer_width;
    u32 framebuffer_height;

    VkAllocationCallbacks* allocator;
    VkInstance instance;


    i32 (*find_memory_index)(u32 required_types, VkMemoryPropertyFlags required_properties);



    f32 frame_delta_time;

    u64 framebuffer_generation;
    u64 framebuffer_last_generation;

    vulkan_buffer object_vertex_buffer;
    vulkan_buffer object_index_buffer;

    vulkan_swapchain swapchain;
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

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debug_utils_messenger;
#endif
    VkSurfaceKHR surface;
    vulkan_device device;

    vulkan_material_shader material_shader;
    vulkan_ui_shader ui_shader;

    // Framebuffers used for world rendering, one per frame
    VkFramebuffer world_framebuffers[3];



    vulkan_geometry_buffer_data geometries[VULKAN_MAX_GEOMETRY_COUNT];
} Context;

#pragma once

#include "defines.h"
#include "third_party/cglm/cglm.h"
#include "resources/resources.h"

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




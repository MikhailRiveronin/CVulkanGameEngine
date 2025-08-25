#pragma once

#include "defines.h"
#include "math/math_types.h"
#include "resources/resource_types.h"

typedef enum renderer_backend_type {
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} renderer_backend_type;

typedef struct global_uniform_data {
    mat4 view;
    mat4 proj;
    mat4 reserved0;
    mat4 reserved1;
} global_uniform_data;

typedef struct material_uniform_data {
    vec4 diffuse_color;
    vec4 reserved0;
    vec4 reserved1;
    vec4 reserved2;
} material_uniform_data;

typedef struct geometry_render_data {
    mat4 world;
    geometry* geometry;
} geometry_render_data;

struct platform_state;
struct texture;

typedef struct renderer_backend {
    struct platform_state* plat_state;
    u64 frameCount;

    b8 (* init)(struct renderer_backend* backend, char const* appName);
    void (* destroy)(struct renderer_backend* backend);

    b8 (* begin_frame)(struct renderer_backend* backend, f64 delta_time);
    b8 (* end_frame)(struct renderer_backend* backend, f64 delta_time);

    void (* resize)(struct renderer_backend* backend, i16 width, i16 height);
    
    void (* update_global_state)(mat4 view, mat4 proj, vec3 view_pos, vec4 ambient_color, i32 mode);
    void (* draw_geometry)(geometry_render_data render_data);

    void (* create_texture)(u8 const* pixels, texture* texture);
    void (* destroy_texture)(texture* texture);

    b8 (* create_material)(material* material);
    void (* destroy_material)(material* material);

    b8 (* create_geometry)(
        geometry* geometry,
        u32 vertex_count,
        vertex_3d const* vertices,
        u32 index_count,
        u32 const* indices);
    void (* destroy_geometry)(geometry* geometry);
} renderer_backend;

typedef struct render_packet {
    f64 delta_time;

    u32 geometry_count;
    geometry_render_data* geometries;
} render_packet;

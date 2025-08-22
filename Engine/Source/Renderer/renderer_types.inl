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

typedef struct object_uniform_data {
    vec4 diffuse_color;
    vec4 reserved0;
    vec4 reserved1;
    vec4 reserved2;
} object_uniform_data;

typedef struct geometry_render_data {
    mat4 world;
    material* material;
} geometry_render_data;

struct platform_state;
struct texture;

typedef struct renderer_backend {
    struct platform_state* plat_state;
    u64 frameCount;

    b8 (* on_init)(struct renderer_backend* backend, char const* appName);
    void (* on_destroy)(struct renderer_backend* backend);

    b8 (* begin_frame)(struct renderer_backend* backend, f64 deltaTime);
    void (* on_update_global_state)(mat4 view, mat4 proj, vec3 view_pos, vec4 ambient_color, i32 mode);
    b8 (* endFrame)(struct renderer_backend* backend, f64 deltaTime);
    void (* on_resize)(struct renderer_backend* backend, i16 width, i16 height);

    void (* on_update_object_state)(geometry_render_data render_data);

    void (* create_texture)(u8 const* pixels, struct texture* texture);
    void (* destroy_texture)(struct texture* texture);
} renderer_backend;

typedef struct render_packet {
    f64 deltaTime;
} render_packet;

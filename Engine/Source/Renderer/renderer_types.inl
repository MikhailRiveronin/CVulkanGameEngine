#pragma once

#include "defines.h"
#include "math/math_types.h"

typedef enum RendererBackendType {
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} RendererBackendType;

typedef struct global_uniform_data {
    mat4 view;
    mat4 proj;
    mat4 reserved0;
    mat4 reserved1;
} global_uniform_data;

struct platform_state;

typedef struct renderer_backend {
    struct platform_state* plat_state;
    u64 frameCount;

    b8 (* on_init)(struct renderer_backend* backend, char const* appName);
    void (* on_destroy)(struct renderer_backend* backend);

    b8 (* begin_frame)(struct renderer_backend* backend, f64 deltaTime);
    void (* on_update_global_state)(mat4 view, mat4 proj, vec3 view_pos, vec4 ambient_color, i32 mode);
    b8 (* endFrame)(struct renderer_backend* backend, f64 deltaTime);
    void (* on_resize)(struct renderer_backend* backend, i16 width, i16 height);

    void (* on_update_object_state)(mat4 world);
} renderer_backend;

typedef struct RenderPacket {
    f64 deltaTime;
} RenderPacket;

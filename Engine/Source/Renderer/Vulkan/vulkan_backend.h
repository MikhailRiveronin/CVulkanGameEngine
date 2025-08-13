#pragma once

#include "vulkan_types.inl"
#include "Renderer/renderer_backend.h"
#include "math/math_types.h"

b8 vulkan_backend_create(renderer_backend* backend, char const* app_name);
void vulkan_backend_destroy(renderer_backend* backend);

b8 vulkan_backend_begin_frame(renderer_backend* backend, f64 deltaTime);
void vulkan_backend_update_global_state(mat4 view, mat4 proj, vec3 view_pos, vec4 ambient_color, i32 mode);
b8 vulkan_backend_end_frame(renderer_backend* backend, f64 deltaTime);

void vulkan_backend_on_resize(renderer_backend* backend, i16 width, i16 height);

void vulkan_backend_update_object_state(mat4 world);

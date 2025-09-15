#pragma once

#include "vulkan_types.h"
#include "renderer/renderer_backend.h"
#include "third_party/cglm/cglm.h"
#include "resources/resource_types.h"

b8 vulkan_backend_create(renderer_backend* backend, char const* app_name);
void vulkan_backend_destroy(renderer_backend* backend);

b8 vulkan_backend_begin_frame(renderer_backend* backend, f64 delta_time);
void vulkan_backend_update_global_state(mat4 view, mat4 proj, vec3 view_pos, vec4 ambient_color, i32 mode);
void vulkan_renderer_update_global_ui_state(mat4 projection, mat4 view, i32 mode);
b8 vulkan_backend_end_frame(renderer_backend* backend, f64 deltaTime);

void vulkan_backend_on_resize(renderer_backend* backend, i16 width, i16 height);

void vulkan_backend_draw_geometry(geometry_render_data render_data);

void vulkan_backend_create_texture(u8 const* pixels, texture* texture);
void vulkan_backend_destroy_texture(texture* texture);

b8 vulkan_backend_create_material(material* material);
void vulkan_backend_destroy_material(material* material);

b8 vulkan_backend_create_geometry(geometry* geometry, u32 vertex_size, u32 vertex_count, void const* vertices, u32 index_size, u32 index_count, u32 const* indices);
void vulkan_backend_destroy_geometry(geometry* geometry);

b8 vulkan_renderer_begin_renderpass(renderer_backend* backend, u8 renderpass_id);
b8 vulkan_renderer_end_renderpass(renderer_backend* backend, u8 renderpass_id);

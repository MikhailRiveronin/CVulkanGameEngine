#pragma once

#include "renderer_types.h"
#include "resources/resources.h"

struct platform_state;

b8 renderer_system_startup(u64* memory_size, void* memory, char const* appName);
void renderer_system_shutdown();

b8 renderer_frontend_draw_frame(render_packet* packet);
void renderer_frontend_resize(i16 width, i16 height);

void renderer_frontend_create_texture(u8 const* pixels, Texture* texture);
void renderer_frontend_destroy_texture(Texture* texture);

// TODO: Remove from export
LIB_API void renderer_frontend_set_view(mat4s view);

b8 renderer_frontend_create_material(Material* material);
void renderer_frontend_destroy_material(Material* material);

b8 renderer_frontend_create_geometry(Geometry* geometry, u32 vertex_size_in_bytes, u32 vertex_count, void const* vertices, u32 index_size_in_bytes, u32 index_count, u32 const* indices);
void renderer_frontend_destroy_geometry(Geometry* geometry);

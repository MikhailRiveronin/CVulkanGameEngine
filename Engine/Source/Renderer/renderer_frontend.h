#pragma once

#include "renderer_types.h"
#include "resources/resource_types.h"

struct platform_state;

b8 renderer_system_startup(u64* memory_size, void* memory, char const* appName);
void renderer_system_shutdown();

b8 renderer_frontend_draw_frame(render_packet* packet);
void renderer_frontend_resize(i16 width, i16 height);

void renderer_frontend_create_texture(u8 const* pixels, texture_resource* texture);
void renderer_frontend_destroy_texture(texture_resource* texture);

// TODO: Remove from export
LIB_API void renderer_frontend_set_view(mat4 view);

b8 renderer_frontend_create_material(material_resource* material);
void renderer_frontend_destroy_material(material_resource* material);

b8 renderer_frontend_create_geometry(geometry_resource* geometry, u32 vertex_size_in_bytes, u32 vertex_count, void const* vertices, u32 index_size_in_bytes, u32 index_count, u32 const* indices);
void renderer_frontend_destroy_geometry(geometry_resource* geometry);

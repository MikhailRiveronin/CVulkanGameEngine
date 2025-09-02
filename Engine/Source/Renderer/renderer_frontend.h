#pragma once

#include "renderer_types.inl"
#include "resources/resource_types.h"

struct platform_state;

b8 renderer_system_startup(u64* memory_size, void* memory, char const* appName);
void renderer_system_shutdown();

b8 renderer_frontend_draw_frame(render_packet* packet);
void renderer_frontend_resize(i16 width, i16 height);

void renderer_frontend_create_texture(u8 const* pixels, texture* texture);
void renderer_frontend_destroy_texture(texture* texture);

// TODO: Remove from export
LIB_API void renderer_frontend_set_view(mat4 view);

b8 renderer_frontend_create_material(material* material);
void renderer_frontend_destroy_material(material* material);

b8 renderer_create_geometry(
    geometry* geometry,
    u32 vertex_size,
    u32 vertex_count,
    void const* vertices,
    u32 index_size,
    u32 index_count,
    u32 const* indices);
void renderer_destroy_geometry(geometry* geometry);

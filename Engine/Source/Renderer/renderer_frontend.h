#pragma once

#include "renderer_types.inl"

struct platform_state;
struct StaticMeshData;

b8 renderer_system_startup(u64* memory_size, void* memory, char const* appName);
void renderer_system_shutdown();

b8 renderer_draw_frame(render_packet* packet);

void renderer_frontend_resize(i16 width, i16 height);

void renderer_create_texture(
    char const* name,
    b8 auto_release,
    i32 width,
    i32 height,
    i32 channel_count,
    u8 const* pixels,
    b8 has_transparency,
    struct texture* texture);
void renderer_destroy_texture(struct texture* texture);

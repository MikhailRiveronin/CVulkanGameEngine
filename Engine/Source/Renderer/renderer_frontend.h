#pragma once

#include "renderer_types.inl"

struct platform_state;
struct StaticMeshData;

b8 renderer_system_startup(u64* memory_size, void* memory, char const* appName);
void renderer_system_shutdown();

b8 renderer_draw_frame(RenderPacket* packet);

void renderer_frontend_resize(i16 width, i16 height);


#pragma once

#include "renderer_types.inl"

struct platform_state;
struct StaticMeshData;

b8 rendererInit(char const* appName, struct platform_state* platformState);
void rendererDestroy();

b8 renderer_draw_frame(RenderPacket* packet);

void renderer_frontend_resize(i16 width, i16 height);


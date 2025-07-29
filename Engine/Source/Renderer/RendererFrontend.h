#pragma once

#include "RendererTypes.inl"

struct PlatformState;
struct StaticMeshData;

b8 rendererInit(char const* appName, struct PlatformState* platformState);
void rendererDestroy();

b8 renderer_draw_frame(RenderPacket* packet);

void renderer_frontend_resize(i16 width, i16 height);


#pragma once

#include "RendererTypes.inl"

struct PlatformState;
struct StaticMeshData;

b8 rendererInit(char const* appName, struct PlatformState* platformState);
void rendererDestroy();

b8 rendererDrawFrame(RenderPacket* packet);

void rendererOnResize(u16 width, u16 height);


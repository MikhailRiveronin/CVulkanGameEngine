#pragma once

#include "Renderer/RendererTypes.h"

struct PlatformState;
struct StaticMeshData;

bool rendererStartup(const char* appName, struct PlatformState* platformState);
void rendererShutdown();

void rendererOnResize(uint16 width, uint16 height);

bool rendererDrawFrame(RenderPacket* packet);

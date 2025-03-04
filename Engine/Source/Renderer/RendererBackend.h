#pragma once

#include "Renderer/RendererTypes.h"

struct PlatformState;

bool rendererBackendCreate(RendererBackendType type, struct PlatformState* platformState, RendererBackend* outBackend);
void rendererBackendDestroy(RendererBackend* backend);

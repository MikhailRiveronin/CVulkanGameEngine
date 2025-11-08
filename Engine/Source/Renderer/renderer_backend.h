#pragma once

#include "renderer_types.h"

b8 renderer_backend_create(renderer_backend_type type, renderer_backend* backend);
void renderer_backend_destroy(renderer_backend* backend);

#pragma once

#include "third_party/cglm/cglm.h"
#include <math.h>

typedef struct vertex_3d {
    vec3 position;
    vec2 tex_coord;
} vertex_3d;

typedef struct vertex_2d {
    vec2 position;
    vec2 tex_coord;
} vertex_2d;

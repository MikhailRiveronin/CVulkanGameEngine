#pragma once

#include "third_party/cglm/struct.h"
#include <math.h>

typedef struct vertex_3d {
    vec3s pos;
    vec2s tex_coord;
} vertex_3d;

typedef struct vertex_2d {
    vec2s pos;
    vec2s tex_coord;
} vertex_2d;

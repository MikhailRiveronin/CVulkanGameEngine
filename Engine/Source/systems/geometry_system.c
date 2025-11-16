#include "geometry_system.h"

#include "core/logger.h"
#include "systems/memory_system.h"
#include "core/string_utils.h"
#include "math/math_types.h"
#include "renderer/renderer_frontend.h"
#include "systems/material_system.h"

typedef struct geometry_reference {
    u64 reference_count;
    Geometry geometry;
    b8 auto_release;
} geometry_reference;

typedef struct geometry_system_state {
    Geometry_System_Config config;

    Geometry default_geometry;
    Geometry default_2d_geometry;

    // Array of registered meshes.
    geometry_reference* geometry_references;
} geometry_system_state;

static geometry_system_state* system_state;

static b8 create_default_geometries(geometry_system_state* state);
static b8 create_geometry(geometry_system_configuration config, Geometry* geometry);
static void destroy_geometry(Geometry* geometry);

b8 geometry_system_startup(u64* required_memory_size_in_bytes, void* memory, Geometry_System_Config config)
{
    if (config.max_geometry_count == 0) {
        LOG_FATAL("geometry_system_startup: config.max_geometry_count must be > 0");
        return FALSE;
    }

    *required_memory_size_in_bytes = sizeof(*system_state) + config.max_geometry_count * sizeof(geometry_reference);
    if (!memory) {
        return TRUE;
    }

    system_state = memory;
    system_state->config = config;
    system_state->geometry_references = (geometry_reference*)((char*)memory + sizeof(*system_state));

    u32 count = system_state->config.max_geometry_count;
    for (u32 i = 0; i < count; ++i) {
        system_state->geometry_references[i].geometry.id = INVALID_ID;
        system_state->geometry_references[i].geometry.internal_id = INVALID_ID;
        system_state->geometry_references[i].geometry.generation = INVALID_ID;
    }

    if (!create_default_geometries(system_state)) {
        LOG_FATAL("geometry_system_startup: Failed to create default geometries");
        return FALSE;
    }

    return TRUE;
}

void geometry_system_shutdown()
{
}

Geometry* geometry_system_acquire_by_id(u32 id)
{
    if (id != INVALID_ID && system_state->geometry_references[id].geometry.id != INVALID_ID) {
        system_state->geometry_references[id].reference_count++;
        return &system_state->geometry_references[id].geometry;
    }

    LOG_ERROR("geometry_system_acquire_by_id: Invalid geometry id. Returning NULL...");
    return 0;
}

Geometry* geometry_system_acquire_from_config(geometry_system_configuration config, b8 auto_release)
{
    for (u32 i = 0; i < system_state->config.max_geometry_count; ++i) {
        if (system_state->geometry_references[i].geometry.id == INVALID_ID) {
            system_state->geometry_references[i].geometry.id = i;
            system_state->geometry_references[i].reference_count = 1;
            system_state->geometry_references[i].auto_release = auto_release;

            Geometry* geometry = &system_state->geometry_references[i].geometry;
            if (!create_geometry(config, geometry)) {
                LOG_ERROR("geometry_system_acquire_from_config: Failed to create geometry. Returning NULL...");
                return 0;
            }

            return geometry;
        }
    }

    LOG_ERROR("geometry_system_acquire_from_config: Unable to obtain free slot for geometry. Returning NULL...");
    return 0;
}

void geometry_system_release(Geometry* geometry)
{
    if (geometry && geometry->id != INVALID_ID) {
        geometry_reference* ref = &system_state->geometry_references[geometry->id];
        ref->reference_count--;

        if (ref->reference_count == 0 && ref->auto_release) {
            ref->auto_release = FALSE;
            destroy_geometry(&ref->geometry);
        }

        return;
    }

    LOG_WARNING("geometry_system_release: Invalid geometry id");
}

Geometry* geometry_system_get_default()
{
    if (system_state) {
        return &system_state->default_geometry;
    }

    LOG_FATAL("geometry_system_get_default: Called before system was initialized. Returning NULL...");
    return 0;
}

Geometry* geometry_system_get_default_2d()
{
    if (system_state) {
        return &system_state->default_2d_geometry;
    }

    LOG_FATAL("geometry_system_get_default_2d: Called before system was initialized. Returning NULL...");
    return 0;
}

b8 create_geometry(geometry_system_configuration config, Geometry* geometry)
{
    if (!renderer_frontend_create_geometry(geometry, config.vertex_size_in_bytes, config.vertex_count, config.vertices, config.index_size_in_bytes, config.index_count, config.indices)) {
        system_state->geometry_references[geometry->id].reference_count = 0;
        system_state->geometry_references[geometry->id].auto_release = FALSE;
        geometry->id = INVALID_ID;
        geometry->internal_id = INVALID_ID;
        geometry->generation = INVALID_ID;
        return FALSE;
    }

    if (string_length(config.material_name) > 0) {
        geometry->material = material_system_acquire(config.material_name);
        if (!geometry->material) {
            geometry->material = material_system_get_default_material();
        }
    }

    return TRUE;
}

void destroy_geometry(Geometry* geometry)
{
    renderer_frontend_destroy_geometry(geometry);
    geometry->id = INVALID_ID;
    geometry->internal_id = INVALID_ID;
    geometry->generation = INVALID_ID;

    string_empty(geometry->name);

    if (geometry->material && string_length(geometry->material->name) > 0) {
        material_system_release(geometry->material->name);
        geometry->material = 0;
    }
}

b8 create_default_geometries(geometry_system_state* state)
{
    vertex_3d verts[4];
    memory_zero(verts, _countof(verts) * sizeof(verts[0]));
    const f32 f = 10.0f;

    verts[0].pos[0] = -0.5 * f;
    verts[0].pos[1] = -0.5 * f;
    verts[0].pos[2] = 0.f;
    verts[0].tex_coord[0] = 0.0f;
    verts[0].tex_coord[1] = 0.0f;

    verts[1].pos[0] = 0.5 * f;
    verts[1].pos[1] = 0.5 * f;
    verts[1].pos[2] = 0.f;
    verts[1].tex_coord[0] = 1.0f;
    verts[1].tex_coord[1] = 1.0f;

    verts[2].pos[0] = -0.5 * f;
    verts[2].pos[1] = 0.5 * f;
    verts[2].pos[2] = 0.f;
    verts[2].tex_coord[0] = 0.0f;
    verts[2].tex_coord[1] = 1.0f;

    verts[3].pos[0] = 0.5 * f;
    verts[3].pos[1] = -0.5 * f;
    verts[3].pos[2] = 0.f;
    verts[3].tex_coord[0] = 1.0f;
    verts[3].tex_coord[1] = 0.0f;

    u32 indices[6] = {0, 1, 2, 0, 3, 1};

    if (!renderer_frontend_create_geometry(&state->default_geometry, sizeof(verts[0]), _countof(verts), verts, sizeof(indices[0]), _countof(indices), indices)) {
        LOG_ERROR("create_default_geometries: Failed to create default geometry");
        return FALSE;
    }

    state->default_geometry.material = material_system_get_default_material();

    vertex_2d verts_2d[4];
    memory_zero(verts_2d, _countof(verts_2d) * sizeof(verts_2d[0]));
    verts_2d[0].pos[0] = -0.5 * f;
    verts_2d[0].pos[1] = -0.5 * f;
    verts_2d[0].tex_coord[0] = 0.0f;
    verts_2d[0].tex_coord[1] = 0.0f;

    verts_2d[1].pos[0] = 0.5 * f;
    verts_2d[1].pos[1] = 0.5 * f;
    verts_2d[1].tex_coord[0] = 1.0f;
    verts_2d[1].tex_coord[1] = 1.0f;

    verts_2d[2].pos[0] = -0.5 * f;
    verts_2d[2].pos[1] = 0.5 * f;
    verts_2d[2].tex_coord[0] = 0.0f;
    verts_2d[2].tex_coord[1] = 1.0f;

    verts_2d[3].pos[0] = 0.5 * f;
    verts_2d[3].pos[1] = -0.5 * f;
    verts_2d[3].tex_coord[0] = 1.0f;
    verts_2d[3].tex_coord[1] = 0.0f;

    // Indices (NOTE: counter-clockwise)
    u32 indices_2d[6] = {2, 1, 0, 3, 0, 1};

    if (!renderer_frontend_create_geometry(&state->default_2d_geometry, sizeof(verts_2d[0]), _countof(verts_2d), verts_2d, sizeof(indices_2d[0]), _countof(indices_2d), indices_2d)) {
        LOG_FATAL("create_default_geometries: Failed to create default 2d geometry");
        return FALSE;
    }

    // state->default_2d_geometry.material = material_system_get_default();
    return TRUE;
}

Geometry_Config geometry_system_generate_plane_config(f32 width, f32 height, u32 x_segment_count, u32 y_segment_count, f32 tile_x, f32 tile_y, char const* name, char const* material_name)
{
    Geometry_Config config;
    config.vertex_size = sizeof(vertex_3d);
    config.vertex_count = x_segment_count * y_segment_count * 4;  // 4 verts per segment
    config.vertices = memory_allocate(sizeof(vertex_3d) * config.vertex_count, MEMORY_TAG_ARRAY);
    config.index_count = x_segment_count * y_segment_count * 6;  // 6 indices per segment
    config.index_size = sizeof(u32);
    config.indices = memory_allocate(sizeof(u32) * config.index_count, MEMORY_TAG_ARRAY);

    // TODO: This generates extra vertices, but we can always deduplicate them later.
    f32 seg_width = width / x_segment_count;
    f32 seg_height = height / y_segment_count;
    f32 half_width = width * 0.5f;
    f32 half_height = height * 0.5f;
    for (u32 y = 0; y < y_segment_count; ++y) {
        for (u32 x = 0; x < x_segment_count; ++x) {
            // Generate vertices
            f32 min_x = (x * seg_width) - half_width;
            f32 min_y = (y * seg_height) - half_height;
            f32 max_x = min_x + seg_width;
            f32 max_y = min_y + seg_height;
            f32 min_uvx = (x / (f32)x_segment_count) * tile_x;
            f32 min_uvy = (y / (f32)y_segment_count) * tile_y;
            f32 max_uvx = ((x + 1) / (f32)x_segment_count) * tile_x;
            f32 max_uvy = ((y + 1) / (f32)y_segment_count) * tile_y;

            u32 v_offset = ((y * x_segment_count) + x) * 4;
            vertex_3d* v0 = &((vertex_3d*)config.vertices)[v_offset + 0];
            vertex_3d* v1 = &((vertex_3d*)config.vertices)[v_offset + 1];
            vertex_3d* v2 = &((vertex_3d*)config.vertices)[v_offset + 2];
            vertex_3d* v3 = &((vertex_3d*)config.vertices)[v_offset + 3];

            v0->pos[0] = min_x;
            v0->pos[1] = min_y;
            v0->tex_coord[0] = min_uvx;
            v0->tex_coord[1] = min_uvy;

            v1->pos[0] = max_x;
            v1->pos[1] = max_y;
            v1->tex_coord[0] = max_uvx;
            v1->tex_coord[1] = max_uvy;

            v2->pos[0] = min_x;
            v2->pos[1] = max_y;
            v2->tex_coord[0] = min_uvx;
            v2->tex_coord[1] = max_uvy;

            v3->pos[0] = max_x;
            v3->pos[1] = min_y;
            v3->tex_coord[0] = max_uvx;
            v3->tex_coord[1] = min_uvy;

            // Generate indices
            u32 i_offset = ((y * x_segment_count) + x) * 6;
            ((u32*)config.indices)[i_offset + 0] = v_offset + 0;
            ((u32*)config.indices)[i_offset + 1] = v_offset + 1;
            ((u32*)config.indices)[i_offset + 2] = v_offset + 2;
            ((u32*)config.indices)[i_offset + 3] = v_offset + 0;
            ((u32*)config.indices)[i_offset + 4] = v_offset + 3;
            ((u32*)config.indices)[i_offset + 5] = v_offset + 1;
        }
    }

    if (name && string_length(name) > 0) {
        string_ncopy(config.name, name, GEOMETRY_MAX_NAME_LENGTH);
    } else {
        string_ncopy(config.name, DEFAULT_GEOMETRY_NAME, GEOMETRY_MAX_NAME_LENGTH);
    }

    if (material_name && string_length(material_name) > 0) {
        string_ncopy(config.material_name, material_name, MAX_MATERIAL_NAME_LENGTH);
    } else {
        string_ncopy(config.material_name, DEFAULT_MATERIAL_NAME, MAX_MATERIAL_NAME_LENGTH);
    }

    return config;
}

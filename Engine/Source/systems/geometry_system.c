#include "geometry_system.h"

#include "core/logger.h"
#include "core/memory_utils.h"
#include "core/string_utils.h"
#include "systems/material_system.h"
#include "renderer/renderer_frontend.h"

typedef struct geometry_reference {
    u64 reference_count;
    geometry geometry;
    b8 auto_release;
} geometry_reference;

typedef struct geometry_system_state {
    geometry_system_config config;

    geometry default_geometry;
    geometry default_2d_geometry;

    // Array of registered meshes.
    geometry_reference* registered_geometries;
} geometry_system_state;

static geometry_system_state* system_state;

b8 create_default_geometries(geometry_system_state* state);
b8 create_geometry(geometry_system_state* state, geometry_config config, geometry* g);
void destroy_geometry(geometry_system_state* state, geometry* g);

b8 geometry_system_startup(u64* state_size_in_bytes, void* memory, geometry_system_config config)
{
    if (config.max_geometry_count == 0) {
        LOG_FATAL("geometry_system_startup - config.max_geometry_count must be > 0.");
        return FALSE;
    }

    // Block of memory will contain state structure, then block for array, then block for hashtable.
    u64 struct_requirement = sizeof(geometry_system_state);
    u64 array_requirement = sizeof(geometry_reference) * config.max_geometry_count;
    *state_size_in_bytes = struct_requirement + array_requirement;

    if (!memory) {
        return TRUE;
    }

    system_state = memory;
    system_state->config = config;

    // The array block is after the state. Already allocated, so just set the pointer.
    void* array_block = (char*)memory + struct_requirement;
    system_state->registered_geometries = array_block;

    // Invalidate all geometries in the array.
    u32 count = system_state->config.max_geometry_count;
    for (u32 i = 0; i < count; ++i) {
        system_state->registered_geometries[i].geometry.id = INVALID_ID;
        system_state->registered_geometries[i].geometry.internal_id = INVALID_ID;
        system_state->registered_geometries[i].geometry.generation = INVALID_ID;
    }

    if (!create_default_geometries(system_state)) {
        LOG_FATAL("Failed to create default geometry. Application cannot continue.");
        return FALSE;
    }

    return TRUE;
}

void geometry_system_shutdown(void* state)
{
    // NOTE: nothing to do here.
}

geometry* geometry_system_acquire_by_id(u32 id)
{
    if (id != INVALID_ID && system_state->registered_geometries[id].geometry.id != INVALID_ID) {
        system_state->registered_geometries[id].reference_count++;
        return &system_state->registered_geometries[id].geometry;
    }

    // NOTE: Should return default geometry instead?
    LOG_ERROR("geometry_system_acquire_by_id cannot load invalid geometry id. Returning nullptr.");
    return 0;
}

geometry* geometry_system_acquire_from_config(geometry_config config, b8 auto_release)
{
    geometry* g = 0;
    for (u32 i = 0; i < system_state->config.max_geometry_count; ++i) {
        if (system_state->registered_geometries[i].geometry.id == INVALID_ID) {
            // Found empty slot.
            system_state->registered_geometries[i].auto_release = auto_release;
            system_state->registered_geometries[i].reference_count = 1;
            g = &system_state->registered_geometries[i].geometry;
            g->id = i;
            break;
        }
    }

    if (!g) {
        LOG_ERROR("Unable to obtain free slot for geometry. Adjust configuration to allow more space. Returning nullptr.");
        return 0;
    }

    if (!create_geometry(system_state, config, g)) {
        LOG_ERROR("Failed to create geometry. Returning nullptr.");
        return 0;
    }

    return g;
}

void geometry_system_release(geometry* geometry)
{
    if (geometry && geometry->id != INVALID_ID) {
        geometry_reference* ref = &system_state->registered_geometries[geometry->id];

        // Take a copy of the id;
        u32 id = geometry->id;
        if (ref->geometry.id == geometry->id) {
            if (ref->reference_count > 0) {
                ref->reference_count--;
            }

            // Also blanks out the geometry id.
            if (ref->reference_count < 1 && ref->auto_release) {
                destroy_geometry(system_state, &ref->geometry);
                ref->reference_count = 0;
                ref->auto_release = FALSE;
            }
        } else {
            LOG_FATAL("Geometry id mismatch. Check registration logic, as this should never occur.");
        }
        return;
    }

    LOG_WARNING("geometry_system_acquire_by_id cannot release invalid geometry id. Nothing was done.");
}

geometry* geometry_system_get_default()
{
    if (system_state) {
        return &system_state->default_geometry;
    }

    LOG_FATAL("geometry_system_get_default called before system was initialized. Returning nullptr.");
    return 0;
}

geometry* geometry_system_get_default_2d() {
    if (system_state) {
        return &system_state->default_2d_geometry;
    }

    LOG_FATAL("geometry_system_get_default_2d called before system was initialized. Returning nullptr.");
    return 0;
}

b8 create_geometry(geometry_system_state* state, geometry_config config, geometry* g)
{
    // Send the geometry off to the renderer to be uploaded to the GPU.
    if (!renderer_create_geometry(g, config.vertex_size, config.vertex_count, config.vertices, config.index_size, config.index_count, config.indices)) {
        // Invalidate the entry.
        state->registered_geometries[g->id].reference_count = 0;
        state->registered_geometries[g->id].auto_release = FALSE;
        g->id = INVALID_ID;
        g->generation = INVALID_ID;
        g->internal_id = INVALID_ID;

        return FALSE;
    }

    // Acquire the material
    if (string_length(config.material_name) > 0) {
        g->material = material_system_acquire_material(config.material_name);
        if (!g->material) {
            g->material = material_system_get_default_material();
        }
    }

    return TRUE;
}

void destroy_geometry(geometry_system_state* state, geometry* g)
{
    renderer_destroy_geometry(g);
    g->internal_id = INVALID_ID;
    g->generation = INVALID_ID;
    g->id = INVALID_ID;

    string_empty(g->name);

    // Release the material.
    if (g->material && string_length(g->material->name) > 0) {
        material_system_release(g->material->name);
        g->material = 0;
    }
}

b8 create_default_geometries(geometry_system_state* state)
{
    vertex_3d verts[4];
    memory_zero(verts, sizeof(vertex_3d) * 4);

    const f32 f = 10.0f;

    verts[0].position.x = -0.5 * f;  // 0    3
    verts[0].position.y = -0.5 * f;  //
    verts[0].position.z = 0.f;
    verts[0].tex_coord.x = 0.0f;      //
    verts[0].tex_coord.y = 0.0f;      // 2    1

    verts[1].position.y = 0.5 * f;
    verts[1].position.x = 0.5 * f;
    verts[1].position.z = 0.f;
    verts[1].tex_coord.x = 1.0f;
    verts[1].tex_coord.y = 1.0f;

    verts[2].position.x = -0.5 * f;
    verts[2].position.y = 0.5 * f;
    verts[2].position.z = 0.f;
    verts[2].tex_coord.x = 0.0f;
    verts[2].tex_coord.y = 1.0f;

    verts[3].position.x = 0.5 * f;
    verts[3].position.y = -0.5 * f;
    verts[3].position.z = 0.f;
    verts[3].tex_coord.x = 1.0f;
    verts[3].tex_coord.y = 0.0f;

    u32 indices[6] = {0, 1, 2, 0, 3, 1};

    // Send the geometry off to the renderer to be uploaded to the GPU.
    if (!renderer_create_geometry(&state->default_geometry, sizeof(vertex_3d), 4, verts, sizeof(u32), 6, indices)) {
        LOG_ERROR("Failed to create default geometry. Application cannot continue.");
        return FALSE;
    }

    // Acquire the default material.
    state->default_geometry.material = material_system_get_default_material();

    // Create default 2d geometry.
    vertex_2d verts2d[4];
    kzero_memory(verts2d, sizeof(vertex_2d) * 4);
    verts2d[0].position.x = -0.5 * f;  // 0    3
    verts2d[0].position.y = -0.5 * f;  //
    verts2d[0].tex_coord.x = 0.0f;      //
    verts2d[0].tex_coord.y = 0.0f;      // 2    1

    verts2d[1].position.y = 0.5 * f;
    verts2d[1].position.x = 0.5 * f;
    verts2d[1].tex_coord.x = 1.0f;
    verts2d[1].tex_coord.y = 1.0f;

    verts2d[2].position.x = -0.5 * f;
    verts2d[2].position.y = 0.5 * f;
    verts2d[2].tex_coord.x = 0.0f;
    verts2d[2].tex_coord.y = 1.0f;

    verts2d[3].position.x = 0.5 * f;
    verts2d[3].position.y = -0.5 * f;
    verts2d[3].tex_coord.x = 1.0f;
    verts2d[3].tex_coord.y = 0.0f;

    // Indices (NOTE: counter-clockwise)
    u32 indices2d[6] = {2, 1, 0, 3, 0, 1};

    // Send the geometry off to the renderer to be uploaded to the GPU.
    if (!renderer_create_geometry(&state->default_2d_geometry, sizeof(vertex_2d), 4, verts2d, sizeof(u32), 6, indices2d)) {
        LOG_FATAL("Failed to create default 2d geometry. Application cannot continue.");
        return FALSE;
    }

    // Acquire the default material.
    state->default_2d_geometry.material = material_system_get_default();

    return TRUE;
}

geometry_config geometry_system_generate_plane_config(f32 width, f32 height, u32 x_segment_count, u32 y_segment_count, f32 tile_x, f32 tile_y, const char* name, const char* material_name) {
    if (width == 0) {
        LOG_WARNING("Width must be nonzero. Defaulting to one.");
        width = 1.0f;
    }
    if (height == 0) {
        LOG_WARNING("Height must be nonzero. Defaulting to one.");
        height = 1.0f;
    }
    if (x_segment_count < 1) {
        LOG_WARNING("x_segment_count must be a positive number. Defaulting to one.");
        x_segment_count = 1;
    }
    if (y_segment_count < 1) {
        LOG_WARNING("y_segment_count must be a positive number. Defaulting to one.");
        y_segment_count = 1;
    }

    if (tile_x == 0) {
        LOG_WARNING("tile_x must be nonzero. Defaulting to one.");
        tile_x = 1.0f;
    }
    if (tile_y == 0) {
        LOG_WARNING("tile_y must be nonzero. Defaulting to one.");
        tile_y = 1.0f;
    }

    geometry_config config;
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

            v0->position.x = min_x;
            v0->position.y = min_y;
            v0->tex_coord.x = min_uvx;
            v0->tex_coord.y = min_uvy;

            v1->position.x = max_x;
            v1->position.y = max_y;
            v1->tex_coord.x = max_uvx;
            v1->tex_coord.y = max_uvy;

            v2->position.x = min_x;
            v2->position.y = max_y;
            v2->tex_coord.x = min_uvx;
            v2->tex_coord.y = max_uvy;

            v3->position.x = max_x;
            v3->position.y = min_y;
            v3->tex_coord.x = max_uvx;
            v3->tex_coord.y = min_uvy;

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
        string_ncopy(config.name, name, GEOMETRY_NAME_MAX_LENGTH);
    } else {
        string_ncopy(config.name, DEFAULT_GEOMETRY_NAME, GEOMETRY_NAME_MAX_LENGTH);
    }

    if (material_name && string_length(material_name) > 0) {
        string_ncopy(config.material_name, material_name, MATERIAL_NAME_MAX_LENGTH);
    } else {
        string_ncopy(config.material_name, DEFAULT_MATERIAL_NAME, MATERIAL_NAME_MAX_LENGTH);
    }

    return config;
}

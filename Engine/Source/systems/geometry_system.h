#pragma once

#include "renderer/renderer_types.h"

typedef struct Geometry_System_Config {
    // Max number of geometries that can be loaded at once.
    // NOTE: Should be significantly greater than the number of static meshes because
    // the there can and will be more than one of these per mesh.
    // Take other systems into account as well
    u32 max_geometry_count;
} Geometry_System_Config;

typedef struct Geometry_Config
{
    u32 vertex_size;
    u32 vertex_count;
    void* vertices;

    u32 index_size;
    u32 index_count;
    void* indices;

    char name[GEOMETRY_MAX_NAME_LENGTH];
    char material_name[MAX_MATERIAL_NAME_LENGTH];
} Geometry_Config;

#define DEFAULT_GEOMETRY_NAME "default"

b8 geometry_system_startup(u64* memory_requirement, void* state, Geometry_System_Config config);
void geometry_system_shutdown();

/**
 * @brief Acquires an existing geometry by id.
 * 
 * @param id The geometry identifier to acquire by.
 * @return A pointer to the acquired geometry or nullptr if failed.
 */
Geometry* geometry_system_acquire_by_id(u32 id);

/**
 * @brief Registers and acquires a new geometry using the given config.
 * 
 * @param config The geometry configuration.
 * @param auto_release Indicates if the acquired geometry should be unloaded when its reference count reaches 0.
 * @return A pointer to the acquired geometry or nullptr if failed. 
 */
Geometry* geometry_system_acquire_from_config(geometry_system_configuration config, b8 auto_release);

/**
 * @brief Releases a reference to the provided geometry.
 * 
 * @param geometry The geometry to be released.
 */
void geometry_system_release(Geometry* geometry);

/**
 * @brief Obtains a pointer to the default geometry.
 * 
 * @return A pointer to the default geometry. 
 */
Geometry* geometry_system_get_default();

/**
 * @brief Obtains a pointer to the default geometry.
 * 
 * @return A pointer to the default geometry. 
 */
Geometry* geometry_system_get_default_2d();

/**
 * @brief Generates configuration for plane geometries given the provided parameters.
 * NOTE: vertex and index arrays are dynamically allocated and should be freed upon object disposal.
 * Thus, this should not be considered production code.
 * 
 * @param width The overall width of the plane. Must be non-zero.
 * @param height The overall height of the plane. Must be non-zero.
 * @param x_segment_count The number of segments along the x-axis in the plane. Must be non-zero.
 * @param y_segment_count The number of segments along the y-axis in the plane. Must be non-zero.
 * @param tile_x The number of times the texture should tile across the plane on the x-axis. Must be non-zero.
 * @param tile_y The number of times the texture should tile across the plane on the y-axis. Must be non-zero.
 * @param name The name of the generated geometry.
 * @param material_name The name of the material to be used.
 * @return A geometry configuration which can then be fed into geometry_system_acquire_from_config().
 */
geometry_system_configuration geometry_system_generate_plane_config(f32 width, f32 height, u32 x_segment_count, u32 y_segment_count, f32 tile_x, f32 tile_y, char const* name, char const* material_name);

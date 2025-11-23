#include "material_system.h"

#include "containers/hash_table.h"
#include "core/logger.h"
#include "core/string_utils.h"
#include "renderer/renderer_frontend.h"
#include "systems/resource_system.h"
#include "systems/texture_system.h"
#include "third_party/cglm/cglm.h"

typedef struct material_system_state {
    Material_System_Config config;

    Material default_material;
    Material* registered_materials;
    hashtable material_reference_table;
} material_system_state;

typedef struct material_reference {
    u64 reference_count;
    u32 handle;
    b8 auto_release;
} material_reference;

static material_system_state* system_state;

static b8 create_default_material();
static b8 load_material(Material_Config config, Material* m);
static void destroy_material(Material* m);

b8 material_system_startup(u64* state_size_in_bytes, void* memory, Material_System_Config config)
{
    if (config.max_material_count == 0) {
        LOG_FATAL("material_system_startup: Failed to startup material system because config.max_material_count == 0");
        return FALSE;
    }

    u64 state_struct_size_in_bytes = sizeof(*system_state);
    u64 array_size_in_bytes = config.max_material_count * sizeof(Material);
    u64 hashtable_requirement = sizeof(material_reference) * config.max_material_count;
    *state_size_in_bytes = state_struct_size_in_bytes + array_size_in_bytes + hashtable_requirement;

    if (!memory) {
        return TRUE;
    }

    system_state = memory;
    system_state->config = config;

    void* array_block = (char*)system_state + state_struct_size_in_bytes;
    system_state->registered_materials = array_block;

    void* hash_table_block = (char*)array_block + array_size_in_bytes;

    hashtable_create(
        sizeof(material_reference),
        config.max_material_count,
        hash_table_block, FALSE,
        &system_state->material_reference_table);

    material_reference invalid_ref;
    invalid_ref.auto_release = FALSE;
    invalid_ref.handle = INVALID_ID;  // Primary reason for needing default values.
    invalid_ref.reference_count = 0;
    hashtable_fill(&system_state->material_reference_table, &invalid_ref);

    for (u32 i = 0; i < system_state->config.max_material_count; ++i) {
        system_state->registered_materials[i].id = INVALID_ID;
        system_state->registered_materials[i].generation = INVALID_ID;
        system_state->registered_materials[i].backend_id = INVALID_ID;
    }

    if (!create_default_material(system_state)) {
        LOG_FATAL("Failed to create default material. Application cannot continue.");
        return FALSE;
    }

    return TRUE;
}

void material_system_shutdown()
{
    if (system_state) {
        for (u32 i = 0; i < system_state->config.max_material_count; ++i) {
            if (system_state->registered_materials[i].id != INVALID_ID) {
                destroy_material(&system_state->registered_materials[i]);
            }
        }

        destroy_material(&system_state->default_material);
    }

    system_state = 0;
}

Material* material_system_acquire(char const* name)
{
    Resource mat_resource;
    if (!resource_system_load(name, RESOURCE_TYPE_MATERIAL, &mat_resource))
    {
        LOG_ERROR("material_system_acquire: Failed to load material resource, returning nullptr");
        return 0;
    }

    Material* material;
    if (mat_resource.data)
    {
        material = material_system_acquire_from_config(*(Material_Config*)mat_resource.data);
    }

    resource_system_unload(&mat_resource);

    if (!material)
    {
        LOG_ERROR("material_system_acquire: Failed to load material resource, returning nullptr");
        return 0;
    }

    return material;
}

Material* material_system_acquire_from_config(Material_Config config)
{
    // Return default material.
    if (string_equali(config.name, DEFAULT_MATERIAL_NAME)) {
        return &system_state->default_material;
    }

    material_reference ref;
    if (system_state && hashtable_get(&system_state->material_reference_table, config.name, &ref)) {
        // This can only be changed the first time a material is loaded.
        if (ref.reference_count == 0) {
            ref.auto_release = config.auto_release;
        }
        ref.reference_count++;

        if (ref.handle == INVALID_ID) {
            // This means no material exists here. Find a free index first.
            u32 count = system_state->config.max_material_count;
            Material* m = 0;
            for (u32 i = 0; i < count; ++i) {
                if (system_state->registered_materials[i].id == INVALID_ID) {
                    // A free slot has been found. Use its index as the handle.
                    ref.handle = i;
                    m = &system_state->registered_materials[i];
                    break;
                }
            }

            // Make sure an empty slot was actually found.
            if (!m || ref.handle == INVALID_ID) {
                LOG_FATAL("material_system_acquire_material - Material system cannot hold anymore materials. Adjust configuration to allow more.");
                return 0;
            }

            // Create new material.
            if (!load_material(config, m)) {
                LOG_ERROR("Failed to load material '%s'.", config.name);
                return 0;
            }

            if (m->generation == INVALID_ID) {
                m->generation = 0;
            } else {
                m->generation++;
            }

            // Also use the handle as the material id.
            m->id = ref.handle;
            LOG_TRACE("Material '%s' does not yet exist. Created, and ref_count is now %i.", config.name, ref.reference_count);
        } else {
            LOG_TRACE("Material '%s' already exists, ref_count increased to %i.", config.name, ref.reference_count);
        }

        // Update the entry.
        hashtable_set(&system_state->material_reference_table, config.name, &ref);
        return &system_state->registered_materials[ref.handle];
    }

    // NOTE: This would only happen in the event something went wrong with the state.
    LOG_ERROR("material_system_acquire_from_config failed to acquire material '%s'. Null pointer will be returned.", config.name);
    return 0;
}

void material_system_release(char const* name)
{
    // Ignore release requests for the default material.
    if (string_equali(name, DEFAULT_MATERIAL_NAME)) {
        return;
    }
    material_reference ref;
    if (system_state && hashtable_get(&system_state->material_reference_table, name, &ref)) {
        if (ref.reference_count == 0) {
            LOG_WARNING("Tried to release non-existent material: '%s'", name);
            return;
        }
        ref.reference_count--;
        if (ref.reference_count == 0 && ref.auto_release) {
            Material* m = &system_state->registered_materials[ref.handle];

            // Destroy/reset material.
            destroy_material(m);

            // Reset the reference.
            ref.handle = INVALID_ID;
            ref.auto_release = FALSE;
            LOG_TRACE("Released material '%s'., Material unloaded because reference count=0 and auto_release=TRUE.", name);
        } else {
            LOG_TRACE("Released material '%s', now has a reference count of '%i' (auto_release=%s).", name, ref.reference_count, ref.auto_release ? "TRUE" : "FALSE");
        }

        // Update the entry.
        hashtable_set(&system_state->material_reference_table, name, &ref);
    } else {
        LOG_ERROR("material_system_release failed to release material '%s'.", name);
    }
}

Material* material_system_get_default_material()
{
    if (system_state) {
        return &system_state->default_material;
    }

    LOG_FATAL("material_system_get_default_material: Called before system_state is initialized");
    return 0;
}

b8 load_material(Material_Config config, Material* m)
{
    memory_zero(m, sizeof(Material));

    // name
    string_ncopy(m->name, config.name, MAX_MATERIAL_NAME_LENGTH);

    m->type = config.type;

    // Diffuse colour
    glm_vec4_copy(config.diffuse_colour, m->diffuse_colour);

    // Diffuse map
    if (string_length(config.diffuse_texture_name) > 0) {
        m->diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
        m->diffuse_map.texture = texture_system_acquire(config.diffuse_texture_name, TRUE);
        if (!m->diffuse_map.texture) {
            LOG_WARNING("Unable to load texture '%s' for material '%s', using default.", config.diffuse_texture_name, m->name);
            m->diffuse_map.texture = texture_system_get_default_texture();
        }
    } else {
        // NOTE: Only set for clarity, as call to memory_zero above does this already.
        m->diffuse_map.use = TEXTURE_USE_UNKNOWN;
        m->diffuse_map.texture = 0;
    }

    // TODO: other maps

    // Send it off to the renderer to acquire resources.
    if (!renderer_frontend_create_material(m)) {
        LOG_ERROR("Failed to acquire renderer resources for material '%s'.", m->name);
        return FALSE;
    }

    return TRUE;
}

void destroy_material(Material* m)
{
    LOG_TRACE("Destroying material '%s'...", m->name);

    // Release texture references.
    if (m->diffuse_map.texture) {
        texture_system_release(m->diffuse_map.texture->name);
    }

    // Release renderer resources.
    renderer_frontend_destroy_material(m);

    // Zero it out, invalidate IDs.
    memory_zero(m, sizeof(Material));
    m->id = INVALID_ID;
    m->generation = INVALID_ID;
    m->backend_id = INVALID_ID;
}

b8 create_default_material()
{
    memory_zero(&system_state->default_material, sizeof(Material));
    system_state->default_material.id = INVALID_ID;
    system_state->default_material.generation = INVALID_ID;

    string_ncopy(system_state->default_material.name, DEFAULT_MATERIAL_NAME, MAX_MATERIAL_NAME_LENGTH);
    glm_vec4_one(system_state->default_material.diffuse_colour);

    system_state->default_material.diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
    system_state->default_material.diffuse_map.texture = texture_system_get_default_texture();

    if (!renderer_frontend_create_material(&system_state->default_material)) {
        LOG_FATAL("create_default_material: Failed to acquire renderer resources for default texture");
        return FALSE;
    }

    return TRUE;
}

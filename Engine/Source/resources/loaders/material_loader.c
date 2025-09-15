#include "material_loader.h"

#include "core/logger.h"
#include "core/memory_utils.h"
#include "core/string_utils.h"
#include "platform/filesystem.h"
#include "resources/resource_types.h"
#include "systems/resource_system.h"
#include "third_party/cglm/cglm.h"

b8 material_loader_load(resource_loader* loader, char const* name, resource* resource);
void material_loader_unload(resource_loader* loader, resource* resource);

resource_loader material_loader_create()
{
    resource_loader loader;
    loader.type = RESOURCE_TYPE_MATERIAL;
    loader.load = material_loader_load;
    loader.unload = material_loader_unload;
    loader.resource_folder_path = "materials";

    return loader;
}

b8 material_loader_load(struct resource_loader* loader, char const* name, resource* resource)
{
    if (!loader || !name || !resource) {
        return FALSE;
    }

    char* format_str = "%s/%s/%s%s";
    char const* extension = ".kmt";
    char complete_path[512];
    string_format(complete_path, format_str, resource_system_asset_folder_path(), loader->resource_folder_path, name, extension);

    resource->complete_path = string_duplicate(complete_path);

    file_handle file;
    if (!filesystem_open(complete_path, ACCESS_MODE_READ, &file)) {
        LOG_ERROR("material_loader_load: Failed to open material file '%s'", complete_path);
        return FALSE;
    }

    material_config* resource_data = memory_allocate(sizeof(*resource_data), MEMORY_TAG_MATERIAL_INSTANCE);
    resource_data->type = MATERIAL_TYPE_WORLD;
    resource_data->auto_release = TRUE;
    resource_data->diffuse_map_name[0] = 0;

    glm_vec4_one(resource_data->diffuse_colour);
    string_ncopy(resource_data->name, name, MATERIAL_NAME_MAX_LENGTH);

    char buffer[512] = "";
    char* p = buffer;
    u64 length;
    u32 line_number = 1;
    while (filesystem_read_line(&file, 511, &p, &length)) {
        char* trimmed = string_trim(buffer);
        length = string_length(trimmed);

        if (length < 1 || trimmed[0] == '#') {
            line_number++;
            continue;
        }

        i32 equal_index = string_index_of(trimmed, '=');
        if (equal_index == -1) {
            LOG_WARNING("material_loader_load: Potential formatting issue found in file '%s': '=' token not found. Skipping line %ui.", complete_path, line_number);
            line_number++;
            continue;
        }

        // Assume a max of 64 characters for the variable name.
        char raw_var_name[64];
        memory_zero(raw_var_name, sizeof(char) * 64);
        string_mid(raw_var_name, trimmed, 0, equal_index);
        char* trimmed_var_name = string_trim(raw_var_name);

        // Assume a max of 511-65 (446) for the max length of the value to account for the variable name and the '='.
        char raw_value[446];
        memory_zero(raw_value, sizeof(char) * 446);
        string_mid(raw_value, trimmed, equal_index + 1, -1);  // Read the rest of the line
        char* trimmed_value = string_trim(raw_value);

        // Process the variable.
        if (string_equali(trimmed_var_name, "version")) {
            // TODO: version
        } else if (string_equali(trimmed_var_name, "name")) {
            string_ncopy(resource_data->name, trimmed_value, MATERIAL_NAME_MAX_LENGTH);
        } else if (string_equali(trimmed_var_name, "diffuse_map_name")) {
            string_ncopy(resource_data->diffuse_map_name, trimmed_value, TEXTURE_NAME_MAX_LENGTH);
        } else if (string_equali(trimmed_var_name, "diffuse_colour")) {
            // Parse the colour
            if (!string_to_vec4(trimmed_value, &resource_data->diffuse_colour)) {
                LOG_WARNING("Error parsing diffuse_colour in file '%s'. Using default of white instead.", complete_path);
                // NOTE: already assigned above, no need to have it here.
            }
        } else if (string_equali(trimmed_var_name, "type")) {
            // TODO: other material types.
            if (string_equali(trimmed_value, "ui")) {
                resource_data->type = MATERIAL_TYPE_UI;
            }
        }

        // Clear the line buffer.
        memory_zero(buffer, sizeof(char) * 512);
        line_number++;
    }

    filesystem_close(&file);

    resource->data = resource_data;
    resource->data_size = sizeof(*resource_data);
    resource->name = name;

    return TRUE;
}

void material_loader_unload(struct resource_loader* loader, resource* resource)
{
    if (!loader || !resource) {
        LOG_WARNING("material_loader_unload: Called with nullptr for loader or resource");
        return;
    }

    u32 path_length = string_length(resource->complete_path);
    if (path_length) {
        memory_free(resource->complete_path, sizeof(char) * path_length + 1, MEMORY_TAG_STRING);
    }

    if (resource->data) {
        memory_free(resource->data, resource->data_size, MEMORY_TAG_MATERIAL_INSTANCE);
        resource->data = 0;
        resource->data_size = 0;
        resource->loader_id = INVALID_ID;
    }
}

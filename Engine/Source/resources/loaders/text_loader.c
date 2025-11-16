#include "text_loader.h"

#include "core/logger.h"
#include "systems/memory_system.h"
#include "core/string_utils.h"
#include "resources/resources.h"
#include "systems/resource_system.h"
#include "math/math_types.h"

#include "platform/filesystem.h"

b8 text_loader_load(struct Resource_Loader* self, char const* name, Resource* out_resource)
{
    if (!self || !name || !out_resource) {
        return FALSE;
    }

    char* format_str = "%s/%s/%s%s";
    char full_file_path[512];
    string_format(full_file_path, format_str, resource_system_asset_folder(), self->resource_type_subfolder, name, "");

    // TODO: Should be using an allocator here.
    out_resource->complete_path = string_duplicate(full_file_path);

    File_Handle f;
    if (!filesystem_open(full_file_path, ACCESS_MODE_READ, &f)) {
        LOG_ERROR("text_loader_load - unable to open file for text reading: '%s'.", full_file_path);
        return FALSE;
    }

    u64 file_size = 0;
    if (!filesystem_size(&f, &file_size)) {
        LOG_ERROR("Unable to text read file: %s.", full_file_path);
        filesystem_close(&f);
        return FALSE;
    }

    // TODO: Should be using an allocator here.
    char* resource_data = memory_allocate(sizeof(char) * file_size, MEMORY_TAG_ARRAY);
    u64 read_size = 0;
    // if (!filesystem_read_all_text(&f, resource_data, &read_size)) {
    //     LOG_ERROR("Unable to text read file: %s.", full_file_path);
    //     filesystem_close(&f);
    //     return FALSE;
    // }

    filesystem_close(&f);

    out_resource->data = resource_data;
    out_resource->data_size = read_size;
    out_resource->name = name;

    return TRUE;
}

void text_loader_unload(struct Resource_Loader* self, Resource* resource) {
    if (!self || !resource) {
        LOG_WARNING("text_loader_unload called with nullptr for self or resource.");
        return;
    }

    u32 path_length = string_length(resource->complete_path);
    if (path_length) {
        memory_free(resource->complete_path, sizeof(char) * path_length + 1, MEMORY_TAG_STRING);
    }

    if (resource->data) {
        memory_free(resource->data, resource->data_size, MEMORY_TAG_ARRAY);
        resource->data = 0;
        resource->data_size = 0;
        resource->loader_id = INVALID_ID;
    }
}

Resource_Loader text_resource_loader_create()
{
    Resource_Loader loader;
    loader.type = RESOURCE_TYPE_TEXT;
    loader.load = text_loader_load;
    loader.unload = text_loader_unload;
    loader.resource_type_subfolder = "";

    return loader;
}

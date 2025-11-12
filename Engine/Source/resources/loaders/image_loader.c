#include "image_loader.h"

#include "core/logger.h"
#include "systems/memory_system.h"
#include "core/string_utils.h"
#include "resources/resource_types.h"
#include "third_party/stb_image.h"

static b8 load(resource_loader* loader, char const* filename, resource* resource);
static void unload(resource_loader* self, resource* resource);

resource_loader image_loader_create()
{
    resource_loader loader;
    loader.type = RESOURCE_TYPE_IMAGE;
    loader.type_str = "textures";
    loader.load = load;
    loader.unload = unload;

    return loader;
}

b8 load(resource_loader* loader, char const* name, resource* resource)
{
    if (!loader || !name || !resource) {
        return FALSE;
    }

    char* format_str = "%s/%s/%s%s";
    char const* extension = ".png";
    char complete_path[512];
    string_format(complete_path, format_str, resource_system_asset_folder_path(), loader->type_str, name, extension);

    const i32 required_channel_count = 4;
    stbi_set_flip_vertically_on_load(TRUE);
    i32 width;
    i32 height;
    i32 channel_count;
    u8* pixels = stbi_load(complete_path, &width, &height, &channel_count, required_channel_count);

    if (!pixels) {
        LOG_ERROR("image_loader_load: Failed to load file '%s'", complete_path);
        return FALSE;
    }

    char const* failure_reason = stbi_failure_reason();
    if (failure_reason) {
        LOG_ERROR("image_loader_load: Load of '%s' failed: %s", complete_path, failure_reason);

        if (pixels) {
            stbi_image_free(pixels);
        }

        return TRUE;
    }

    image_resource_data* resource_data = memory_allocate(sizeof(*resource_data), MEMORY_TAG_TEXTURE);
    resource_data->pixels = pixels;
    resource_data->width = width;
    resource_data->height = height;
    resource_data->channel_count = channel_count;

    resource->name = name;
    resource->loader_id = loader->id;
    resource->complete_path = string_duplicate(complete_path);
    resource->data_size = sizeof(*resource_data);
    resource->data = resource_data;

    return TRUE;
}

void unload(resource_loader* loader, resource* resource)
{
    if (!loader || !resource) {
        LOG_WARNING("image_loader_unload: Nothing to unload");
        return;
    }

    u32 length = string_length(resource->complete_path);
    if (length) {
        memory_free(resource->complete_path, sizeof(char) * length + 1, MEMORY_TAG_STRING);
    }

    if (resource->data) {
        memory_free(resource->data, resource->data_size, MEMORY_TAG_TEXTURE);
        resource->loader_id = INVALID_ID;
        resource->data_size = 0;
        resource->data = 0;
    }
}

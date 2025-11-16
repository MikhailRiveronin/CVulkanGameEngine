#include "image_loader.h"

#include "core/logger.h"
#include "core/string_utils.h"
#include "resources/resources.h"
#include "systems/memory_system.h"
#include "third_party/stb_image.h"

static b8 load_image(Resource_Loader* loader, char const* name, Resource* resource);
static void unload_image(Resource_Loader* self, Resource* resource);

Resource_Loader image_loader_create()
{
    Resource_Loader loader;
    loader.type = RESOURCE_TYPE_IMAGE;
    loader.resource_type_subfolder = "textures";
    loader.load = load_image;
    loader.unload = unload_image;

    return loader;
}

b8 load_image(Resource_Loader* loader, char const* name, Resource* resource)
{
    if (!loader || !name || !resource)
    {
        return FALSE;
    }

    char complete_path[512];
    string_format(complete_path, "%s/%s/%s", resource_system_asset_folder(), loader->resource_type_subfolder, name);

    const i32 required_channel_count = 4;
    i32 width;
    i32 height;
    i32 channel_count;
    u8* pixels = stbi_load(complete_path, &width, &height, &channel_count, required_channel_count);
    if (!pixels)
    {
        LOG_ERROR("load: Failed to load image '%s'", complete_path);
        return FALSE;
    }

    image_resource_data* resource_data = memory_system_allocate(sizeof(*resource_data), MEMORY_TAG_TEXTURE);
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

void unload_image(Resource_Loader* loader, Resource* resource)
{
    if (!loader || !resource)
    {
        LOG_WARNING("unload: Failed to unload");
        return;
    }

    u32 length = string_length(resource->complete_path);
    if (length > 0)
    {
        memory_system_free(resource->complete_path, sizeof(char) * length + 1, MEMORY_TAG_STRING);
    }

    if (resource->data)
    {
        memory_free(resource->data, resource->data_size, MEMORY_TAG_TEXTURE);
        resource->loader_id = INVALID_ID;
        resource->data_size = 0;
        resource->data = 0;
    }
}

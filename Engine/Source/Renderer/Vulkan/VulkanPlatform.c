#include "VulkanPlatform.h"
#include "containers/darray.h"

void vulkan_platform_get_required_extensions(DARRAY_CSTRING* extensions)
{
#ifdef _WIN32
    DARRAY_PUSH(*extensions, VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
}

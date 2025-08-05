#include "VulkanPlatform.h"
#include "Containers/darray.h"

void vulkanPlatformGetRequiredExtensions(DARRAY_CSTRING* extensions)
{
#ifdef _WIN32
    DARRAY_PUSH(*extensions, VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
}

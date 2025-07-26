#include "VulkanBackend.h"
#include "VulkanPlatform.h"
#include "VulkanDevice.h"
#include "Containers/DArray.h"
#include "Core/Logger.h"
#include "Core/String.h"


static VulkanContext context;

#ifdef _DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    VkDebugUtilsMessengerCallbackDataEXT const* callbackData,
    void* userData);
#endif

b8 vulkanBackendInit(struct RendererBackend* backend, char const* appName, struct PlatformState* platformState)
{
    context.allocator = NULL;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = "Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    DARRAY_DEFINE(char const*, requiredLayers, 0, MEMORY_TAG_STRING);
#ifdef _DEBUG
    DARRAY_PUSH(requiredLayers, "VK_LAYER_KHRONOS_validation");

    LOG_DEBUG("Required layers:");
    for (u32 i = 0; i < requiredLayers.size; ++i) {
        LOG_DEBUG("    %s", DARRAY_AT(requiredLayers, i));
    }
#endif
    {
        u32 propertyCount;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&propertyCount, NULL));
        DARRAY_DEFINE(VkLayerProperties, availableLayers, propertyCount, MEMORY_TAG_RENDERER);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&propertyCount, availableLayers.data));
        availableLayers.size = propertyCount;

        for (u32 i = 0; i < requiredLayers.size; ++i) {
            b8 found = FALSE;
            for (u32 j = 0; j < availableLayers.size; ++j) {
                if (stringEqual(DARRAY_AT(requiredLayers, i), DARRAY_AT(availableLayers, j).layerName)) {
                    found = TRUE;
                    break;
                }
            }

            if (!found) {
                LOG_FATAL("Required validation layer '%s' not found", DARRAY_AT(requiredLayers, i));
                return FALSE;
            }
        }
        DARRAY_DESTROY(availableLayers);
    }

    DARRAY_CSTRING requiredExtensions;
    DARRAY_INIT(requiredExtensions, MEMORY_TAG_STRING);
    DARRAY_PUSH(requiredExtensions, VK_KHR_SURFACE_EXTENSION_NAME);
    vulkanPlatformGetRequiredExtensions(&requiredExtensions);
#ifdef _DEBUG
    DARRAY_PUSH(requiredExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    LOG_DEBUG("Required extensions:");
    for (u32 i = 0; i < requiredExtensions.size; ++i) {
        LOG_DEBUG("    %s", DARRAY_AT(requiredExtensions, i));
    }
#endif
    {
        u32 propertyCount;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, NULL));
        DARRAY_DEFINE(VkExtensionProperties, availableExtensions, propertyCount, MEMORY_TAG_RENDERER);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, availableExtensions.data));
        availableExtensions.size = propertyCount;

        for (u32 i = 0; i < requiredExtensions.size; ++i) {
            b8 found = FALSE;
            for (u32 j = 0; j < availableExtensions.size; ++j) {
                if (stringEqual(DARRAY_AT(requiredExtensions, i), DARRAY_AT(availableExtensions, j).extensionName)) {
                    found = TRUE;
                    break;
                }
            }

            if (!found) {
                LOG_FATAL("Required extension '%s' not found", DARRAY_AT(requiredExtensions, i));
                return FALSE;
            }
        }
        DARRAY_DESTROY(availableExtensions);
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = requiredLayers.size;
    instanceCreateInfo.ppEnabledLayerNames = requiredLayers.data;
    instanceCreateInfo.enabledExtensionCount = requiredExtensions.size;
    instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data;
    VK_CHECK(vkCreateInstance(&instanceCreateInfo, context.allocator, &context.instance));
    LOG_INFO("Vulkan instance created");
    DARRAY_DESTROY(requiredLayers);
    DARRAY_DESTROY(requiredExtensions);

#ifdef _DEBUG
    VkDebugUtilsMessageSeverityFlagsEXT severity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    // severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    VkDebugUtilsMessageTypeFlagsEXT messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    // messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
    debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerCreateInfo.pNext = NULL;
    debugMessengerCreateInfo.flags = 0;
    debugMessengerCreateInfo.messageSeverity = severity;
    debugMessengerCreateInfo.messageType = messageType;
    debugMessengerCreateInfo.pfnUserCallback = debugUtilsMessengerCallback;
    debugMessengerCreateInfo.pUserData = NULL;

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    ASSERT_MSG(func, "Failed to load vkCreateDebugUtilsMessengerEXT");
    VK_CHECK(func(context.instance, &debugMessengerCreateInfo, context.allocator, &context.debugUtilsMessenger));
    LOG_INFO("Debug messenger created");
#endif

    if (!platformCreateVulkanSurface(platformState, &context)) {
        LOG_ERROR("Failed to create surface");
        return FALSE;
    }

    if (!vulkanDeviceInit(&context)) {
        LOG_ERROR("Failed to create device");
        return FALSE;
    }

    LOG_INFO("Vulkan renderer initialized");
    return TRUE;
}

void vulkanBackendDestroy(struct RendererBackend* backend)
{
    vulkanDeviceDestroy(&context);

    vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
#ifdef _DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
    ASSERT_MSG(func, "Failed to load vkDestroyDebugUtilsMessengerEXT");
    func(context.instance, context.debugUtilsMessenger, context.allocator);
#endif
    vkDestroyInstance(context.instance, context.allocator);
}

b8 vulkanBackendBeginFrame(struct RendererBackend* backend, f64 deltaTime)
{
    return TRUE;
}

b8 vulkanBackendEndFrame(struct RendererBackend* backend, f64 deltaTime)
{
    return TRUE;
}

void vulkanBackendResize(struct RendererBackend* backend, u16 width, u16 height)
{
}

#ifdef _DEBUG
VkBool32 debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    VkDebugUtilsMessengerCallbackDataEXT const* callbackData,
    void* userData)
{
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_ERROR("%s", callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARNING("%s", callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_INFO("%s", callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG_TRACE("%s", callbackData->pMessage);
            break;
    }
    return VK_FALSE;
}
#endif

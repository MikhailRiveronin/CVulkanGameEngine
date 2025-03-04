#include "Renderer/Vulkan/VulkanBackend.h"

#include "Core/Logger.h"

#include "Containers/DynamicArray.h"

#include "Renderer/Vulkan/VulkanTypes.h"

static VulkanContext context;

bool vulkanBackendInitialize(RendererBackend* pBackend, const char* pAppName, struct PlatformState* pPlatformState)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = "Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    const char** ppLayerNames = NULL;
#ifdef _DEBUG
    ppLayerNames = DYNAMIC_ARRAY_CREATE(const char*);
    DYNAMIC_ARRAY_PUSH_BACK(ppLayerNames, "VK_LAYER_KHRONOS_validation");
#endif

    {
        uint32 propertyCount;
        CHECK_IF_VK_SUCCESS(vkEnumerateInstanceLayerProperties(&propertyCount, NULL), __FILE__, __LINE__, "Failed to enumerate instance layer properties");
        VkLayerProperties* pProperties = DYNAMIC_ARRAY_CREATE(VkExtensionProperties);
        DYNAMIC_ARRAY_RESERVE(pProperties, propertyCount);
        CHECK_IF_VK_SUCCESS(vkEnumerateInstanceLayerProperties(&propertyCount, pProperties), __FILE__, __LINE__, "Failed to enumerate instance layer properties");

        for (uint32 i = 0; i < DYNAMIC_ARRAY_FIELD_SIZE(ppLayerNames); ++i)
        {
            bool isFound = FALSE;
            for (uint32 j = 0; j < propertyCount; ++j)
            {
                if (strcmp(ppLayerNames[i], pProperties[j].layerName) == 0)
                {
                    isFound = TRUE;
                    break;
                }
            }

            if (isFound)
                continue;

            LOG_ERROR("Layer '%s' is not supported", ppLayerNames[i]);

            return FALSE;
        }
    }

    const char** ppExtensionNames = DYNAMIC_ARRAY_CREATE(const char*);
    DYNAMIC_ARRAY_PUSH_BACK(ppExtensionNames, VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef _WIN32
    DYNAMIC_ARRAY_PUSH_BACK(ppExtensionNames, "VK_KHR_win32_surface");
#endif
#ifdef _DEBUG
    DYNAMIC_ARRAY_PUSH_BACK(ppExtensionNames, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    {
        uint32 propertyCount;
        CHECK_IF_VK_SUCCESS(vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, NULL), __FILE__, __LINE__, "Failed to enumerate instance extension properties");
        VkExtensionProperties* pProperties = DYNAMIC_ARRAY_CREATE(VkExtensionProperties);
        DYNAMIC_ARRAY_RESERVE(pProperties, propertyCount);
        CHECK_IF_VK_SUCCESS(vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, pProperties), __FILE__, __LINE__, "Failed to enumerate instance extension properties");

        for (uint32 i = 0; i < DYNAMIC_ARRAY_FIELD_SIZE(ppExtensionNames); ++i)
        {
            bool isFound = FALSE;
            for (uint32 j = 0; j < propertyCount; ++j)
            {
                if (strcmp(ppExtensionNames[i], pProperties[j].extensionName) == 0)
                {
                    isFound = TRUE;
                    break;
                }
            }

            if (isFound)
                continue;

            LOG_ERROR("Extension '%s' is not supported", ppExtensionNames[i]);

            return FALSE;
        }
    }

#if LOG_DEBUG_ENABLED == 1
    LOG_INFO("Required Vulkan extensions: ");
    for (uint32 i = 0; i < DYNAMIC_ARRAY_FIELD_SIZE(ppExtensionNames); ++i)
    {
        LOG_INFO("                            %s", ppExtensionNames[i]);
    }
#endif

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = DYNAMIC_ARRAY_FIELD_SIZE(ppLayerNames);
    instanceCreateInfo.ppEnabledLayerNames = ppLayerNames;
    instanceCreateInfo.enabledExtensionCount = DYNAMIC_ARRAY_FIELD_SIZE(ppExtensionNames);
    instanceCreateInfo.ppEnabledExtensionNames = ppExtensionNames;

    context.allocator = NULL;
    VkResult result = vkCreateInstance(&instanceCreateInfo, context.allocator, &context.instance);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create Vulkan instance");
        return FALSE;
    }

    return TRUE;
}

void vulkanBackendTerminate(struct RendererBackend* backend)
{
}

bool vulkanBackendBeginFrame(struct RendererBackend* backend, float deltaTime)
{
    return TRUE;
}

bool vulkanBackendEndFrame(struct RendererBackend* backend, float deltaTime)
{
    return TRUE;
}

void vulkanBackendResize(struct RendererBackend* backend, uint16 width, uint16 height)
{
}

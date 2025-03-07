#include "Renderer/Vulkan/VulkanDevice.h"

#include "Containers/DynamicArray.h"

#include "Core/Logger.h"
#include "Core/Memory.h"

typedef struct VulkanPhysicalDeviceRequirements
{
    bool graphics;
    bool present;
    bool compute;
    bool transfer;
    // darray
    const char** deviceExtensionNames;
    bool samplerAnisotropy;
    bool discreteGpu;
} VulkanPhysicalDeviceRequirements;

typedef struct VulkanPhysicalDeviceQueueFamilyInfo
{
    uint32 graphicsFamilyIndex;
    uint32 presentFamilyIndex;
    uint32 computeFamilyIndex;
    uint32 transferFamilyIndex;
} VulkanPhysicalDeviceQueueFamilyInfo;

static bool selectPhysicalDevice(VulkanState* state);
static bool checkIfPhysicalDeviceMeetsRequirements(VkPhysicalDevice device, VkSurfaceKHR surface, const VkPhysicalDeviceProperties* pProperties, const VkPhysicalDeviceFeatures* pFeatures, const VulkanPhysicalDeviceRequirements* pRequirements, VulkanPhysicalDeviceQueueFamilyInfo* pOutQueueFamilyInfo, VulkanSwapchainSupportInfo* out_swapchain_support);

bool vulkanCreateDevice(VulkanState* state)
{
    if (!selectPhysicalDevice(state))
        return FALSE;

    return TRUE;
}

void vulkanDestroyDevice(VulkanState* state)
{
    
}

void vulkanDeviceQuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VulkanSwapchainSupportInfo* pOutSwapchainSupportInfo)
{
    CHECK_IF_VK_SUCCESS(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &pOutSwapchainSupportInfo->surfaceCapabilities), __FILE__, __LINE__, "");

    CHECK_IF_VK_SUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &pOutSwapchainSupportInfo->surfaceFormatCount, NULL), __FILE__, __LINE__, "");
    if (pOutSwapchainSupportInfo->surfaceFormatCount != 0)
    {
        pOutSwapchainSupportInfo->pSurfaceFormats = memoryAllocate(pOutSwapchainSupportInfo->surfaceFormatCount * sizeof(*pOutSwapchainSupportInfo->pSurfaceFormats), MEMORY_TAG_RENDERER);
        CHECK_IF_VK_SUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &pOutSwapchainSupportInfo->surfaceFormatCount, pOutSwapchainSupportInfo->pSurfaceFormats), __FILE__, __LINE__, "");
    }

    CHECK_IF_VK_SUCCESS(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &pOutSwapchainSupportInfo->presentModeCount, NULL), __FILE__, __LINE__, "");
    if (pOutSwapchainSupportInfo->presentModeCount != 0)
    {
        pOutSwapchainSupportInfo->pPresentModes = memoryAllocate(pOutSwapchainSupportInfo->presentModeCount * sizeof(*pOutSwapchainSupportInfo->pPresentModes), MEMORY_TAG_RENDERER);
        CHECK_IF_VK_SUCCESS(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &pOutSwapchainSupportInfo->presentModeCount, pOutSwapchainSupportInfo->pPresentModes), __FILE__, __LINE__, "");
    }
}

bool selectPhysicalDevice(VulkanState* state)
{
    uint32 deviceCount = 0;
    CHECK_IF_VK_SUCCESS(vkEnumeratePhysicalDevices(state->instance, &deviceCount, NULL), __FILE__, __LINE__, "Failed to enumerate physical devices");

    if (deviceCount == 0)
    {
        LOG_FATAL("No devices which support Vulkan were found.");
        return FALSE;
    }

    VkPhysicalDevice* devices = memoryAllocate(deviceCount * sizeof(*devices), MEMORY_TAG_RENDERER);
    CHECK_IF_VK_SUCCESS(vkEnumeratePhysicalDevices(state->instance, &deviceCount, devices), __FILE__, __LINE__, "Failed to enumerate physical devices");

    for (uint32 i = 0; i < deviceCount; ++i)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(devices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(devices[i], &features);

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(devices[i], &memoryProperties);

        VulkanPhysicalDeviceRequirements requirements = {};
        requirements.graphics = TRUE;
        requirements.present = TRUE;
        requirements.transfer = TRUE;
        // NOTE: Enable this if compute will be required.
        // requirements.compute = TRUE;
        requirements.samplerAnisotropy = TRUE;
        requirements.discreteGpu = TRUE;
        requirements.deviceExtensionNames = DYNAMIC_ARRAY_CREATE(const char*);
        const char* extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        DYNAMIC_ARRAY_PUSH_BACK(requirements.deviceExtensionNames, extensionName);

        VulkanPhysicalDeviceQueueFamilyInfo queueFamilyInfo = {};
        bool result = checkIfPhysicalDeviceMeetsRequirements(state->device.physicalDevice, state->surface, &properties, &features, &requirements, &queueFamilyInfo, &state->device.swapchainSupportInfo);

        if (result)
        {
            state->device.physicalDevice = devices[i];
            state->device.graphicsFamilyIndex = queueFamilyInfo.graphicsFamilyIndex;
            state->device.presentFamilyIndex = queueFamilyInfo.presentFamilyIndex;
            state->device.transferFamilyIndex = queueFamilyInfo.transferFamilyIndex;
            // NOTE: set compute index here if needed.

            // Keep a copy of properties, features and memory info for later use.
            state->device.properties = properties;
            state->device.features = features;
            state->device.memoryProperties = memoryProperties;
            break;
        }
    }

    memoryFree(devices, deviceCount * sizeof(*devices), MEMORY_TAG_RENDERER);

    return TRUE;
}

bool checkIfPhysicalDeviceMeetsRequirements(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const VkPhysicalDeviceProperties* pProperties, const VkPhysicalDeviceFeatures* pFeatures, const VulkanPhysicalDeviceRequirements* pRequirements, VulkanPhysicalDeviceQueueFamilyInfo* pOutQueueFamilyInfo, VulkanSwapchainSupportInfo* pOutSwapchainSupportInfo)
{
    pOutQueueFamilyInfo->graphicsFamilyIndex = -1;
    pOutQueueFamilyInfo->presentFamilyIndex = -1;
    pOutQueueFamilyInfo->computeFamilyIndex = -1;
    pOutQueueFamilyInfo->transferFamilyIndex = -1;

    // Discrete GPU?
    if (pRequirements->discreteGpu)
    {
        if (pProperties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            LOG_INFO("Device is not a discrete GPU, and one is required. Skipping the device.");
            return FALSE;
        }
    }

    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamilies = memoryAllocate(queueFamilyCount * sizeof(*queueFamilies), MEMORY_TAG_RENDERER);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

    uint8 minTransferScore = 255;
    for (uint32 i = 0; i < queueFamilyCount; ++i)
    {
        uint8 currentTransferScore = 0;

        // Graphics queue?
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            pOutQueueFamilyInfo->graphicsFamilyIndex = i;
            ++currentTransferScore;
        }

        // Compute queue?
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            pOutQueueFamilyInfo->computeFamilyIndex = i;
            ++currentTransferScore;
        }

        // Transfer queue?
        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            // Take the index if it is the current lowest. This increases the
            // likelihood that it is a dedicated transfer queue.
            if (currentTransferScore <= minTransferScore)
            {
                minTransferScore = currentTransferScore;
                pOutQueueFamilyInfo->transferFamilyIndex = i;
            }
        }

        // Present queue?
        VkBool32 isPresentationSupported = VK_FALSE;
        CHECK_IF_VK_SUCCESS(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &isPresentationSupported), __FILE__, __LINE__, "Failed to get physical device surface support");
        if (isPresentationSupported)
            pOutQueueFamilyInfo->presentFamilyIndex = i;
    }

    memoryFree(queueFamilies, queueFamilyCount * sizeof(*queueFamilies), MEMORY_TAG_RENDERER);

    if ((!pRequirements->graphics || (pRequirements->graphics && pOutQueueFamilyInfo->graphicsFamilyIndex != -1)) && (!pRequirements->present || (pRequirements->present && pOutQueueFamilyInfo->presentFamilyIndex != -1)) && (!pRequirements->compute || (pRequirements->compute && pOutQueueFamilyInfo->computeFamilyIndex != -1)) && (!pRequirements->transfer || (pRequirements->transfer && pOutQueueFamilyInfo->transferFamilyIndex != -1)))
    {
        vulkanDeviceQuerySwapchainSupport(physicalDevice, surface, pOutSwapchainSupportInfo);

        if (pOutSwapchainSupportInfo->surfaceFormatCount < 1 || pOutSwapchainSupportInfo->presentModeCount < 1)
        {
            if (pOutSwapchainSupportInfo->pSurfaceFormats)
                memoryFree(pOutSwapchainSupportInfo->pSurfaceFormats, pOutSwapchainSupportInfo->surfaceFormatCount * sizeof(*pOutSwapchainSupportInfo->pSurfaceFormats), MEMORY_TAG_RENDERER);

            if (pOutSwapchainSupportInfo->presentModeCount)
                memoryFree(pOutSwapchainSupportInfo->pPresentModes, pOutSwapchainSupportInfo->presentModeCount * sizeof(*pOutSwapchainSupportInfo->pPresentModes), MEMORY_TAG_RENDERER);

            LOG_INFO("Required swapchain support not present, skipping device.");

            return FALSE;
        }

        uint32_t availableExtensionCount = 0;
        VkExtensionProperties* pAvailableExtensions;
        CHECK_IF_VK_SUCCESS(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &availableExtensionCount, NULL), __FILE__, __LINE__, "");
        if (availableExtensionCount != 0)
        {
            pAvailableExtensions = memoryAllocate(availableExtensionCount * sizeof(*pAvailableExtensions), MEMORY_TAG_RENDERER);
            CHECK_IF_VK_SUCCESS(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &availableExtensionCount, pAvailableExtensions), __FILE__, __LINE__, "");

            for (uint32 i = 0; i < DYNAMIC_ARRAY_FIELD_SIZE(pRequirements->deviceExtensionNames); ++i)
            {
                bool isFound = FALSE;
                for (uint32 j = 0; j < availableExtensionCount; ++j)
                {
                    if (strcmp(pRequirements->deviceExtensionNames[i], pAvailableExtensions[j].extensionName) == 0)
                    {
                        isFound = TRUE;
                        break;
                    }
                }

                if (!isFound)
                {
                    LOG_INFO("Required device extension not found");
                    memoryFree(pAvailableExtensions, availableExtensionCount * sizeof(*pAvailableExtensions), MEMORY_TAG_RENDERER);
                    return FALSE;
                }
            }

            memoryFree(pAvailableExtensions, availableExtensionCount * sizeof(*pAvailableExtensions), MEMORY_TAG_RENDERER);
        }

        if (pRequirements->samplerAnisotropy && !pFeatures->samplerAnisotropy)
        {
            LOG_INFO("Device does not support samplerAnisotropy, skipping.");
            return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}

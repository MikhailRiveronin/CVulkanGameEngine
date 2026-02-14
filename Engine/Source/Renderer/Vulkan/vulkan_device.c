#include "vulkan_device.h"
#include "containers/darray.h"
#include "core/logger.h"
#include "systems/memory_system.h"
#include "core/string_utils.h"

typedef struct PhysicalDeviceRequirements {
    DARRAY_CSTRING deviceExtensionNames;
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    b8 samplerAnisotropy;
    b8 discreteGpu;
} PhysicalDeviceRequirements;

typedef struct PhysicalDeviceQueueFamilyInfo {
    u32 graphicsFamilyIndex;
    u32 presentFamilyIndex;
    u32 computeFamilyIndex;
    u32 transferFamilyIndex;
} PhysicalDeviceQueueFamilyInfo;

static b8 selectPhysicalDevice(vulkan_context* state);
static b8 physicalDeviceMeetsRequirements(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkPhysicalDeviceProperties const* properties,
    VkPhysicalDeviceFeatures const* features,
    PhysicalDeviceRequirements const* requirements,
    PhysicalDeviceQueueFamilyInfo* queueFamilyInfo,
    VulkanSwapchainSupportInfo* swapchainSupportInfo);
static b8 find_supported_format(
    VkPhysicalDevice physical_device,
    VkImageTiling tiling,
    VkFormatFeatureFlags format_features,
    VkFormat const* candidates,
    u32 candidate_count,
    VkFormat* format);

b8 vulkan_device_create(vulkan_context* context)
{
    if (!selectPhysicalDevice(context)) {
        return FALSE;
    }

    LOG_INFO("Creating logical device...");
    u32 indexCount = 1;
    b8 presentSharesGraphics = context->device.queues.graphics.index == context->device.queues.present.index;
    if (!presentSharesGraphics) {
        indexCount++;
    }

    b8 transferSharesGraphics = context->device.queues.graphics.index == context->device.queues.transfer.index;
    if (!transferSharesGraphics) {
        indexCount++;
    }

    DARRAY_DEFINE(u32, indices, indexCount, MEMORY_TAG_RENDERER);
    u8 index = 0;
    DARRAY_AT(indices, index) = context->device.queues.graphics.index;
    if (!presentSharesGraphics) {
        index++;
        DARRAY_AT(indices, index) = context->device.queues.present.index;
    }

    if (!transferSharesGraphics) {
        index++;
        DARRAY_AT(indices, index) = context->device.queues.transfer.index;
    }

    DARRAY_DEFINE(VkDeviceQueueCreateInfo, queueCreateInfos, indexCount, MEMORY_TAG_RENDERER);
    f32 queuePriority = 1.f;
    for (u8 i = 0; i < indexCount; ++i) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = NULL;
        queueCreateInfo.flags = 0;
        queueCreateInfo.queueFamilyIndex = DARRAY_AT(indices, i);
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        DARRAY_PUSH(queueCreateInfos, queueCreateInfo);
    }

    // TODO: should be configurable
    DARRAY_CSTRING requiredDeviceExtensions;
    DARRAY_INIT(requiredDeviceExtensions, MEMORY_TAG_STRING);
    DARRAY_PUSH(requiredDeviceExtensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // TODO: should be configurable
    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = NULL;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = NULL;
    deviceCreateInfo.enabledExtensionCount = requiredDeviceExtensions.size;
    deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data;
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    VULKAN_CHECK_RESULT(vkCreateDevice(
        context->device.physical_device,
        &deviceCreateInfo,
        context->allocator,
        &context->device.handle));
    LOG_INFO("Logical device created");

    DARRAY_DESTROY(indices);
    DARRAY_DESTROY(queueCreateInfos);

    vkGetDeviceQueue(context->device.handle, context->device.queues.graphics.index, 0, &context->device.queues.graphics.handle);
    vkGetDeviceQueue(context->device.handle, context->device.queues.present.index, 0, &context->device.queues.present.handle);
    vkGetDeviceQueue(context->device.handle, context->device.queues.transfer.index, 0, &context->device.queues.transfer.handle);
    LOG_INFO("Queues obtained");

    VkCommandPoolCreateInfo commandPoolcreateInfo = {};
    commandPoolcreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolcreateInfo.pNext = NULL;
    commandPoolcreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolcreateInfo.queueFamilyIndex = context->device.queues.graphics.index;
    VULKAN_CHECK_RESULT(vkCreateCommandPool(
        context->device.handle,
        &commandPoolcreateInfo,
        context->allocator,
        &context->device.graphics_command_pool));
    LOG_INFO("Graphics command pool obtained");

    return TRUE;
}

void vulkanDeviceDestroy(vulkan_context* context)
{
    vkDestroyCommandPool(context->device.handle, context->device.graphics_command_pool, context->allocator);

    context->device.queues.graphics.index = -1;
    context->device.queues.compute.index = -1;
    context->device.queues.transfer.index = -1;
    context->device.queues.present.index = -1;

    context->device.queues.graphics.handle = VK_NULL_HANDLE;
    context->device.queues.compute.handle = VK_NULL_HANDLE;
    context->device.queues.transfer.handle = VK_NULL_HANDLE;
    context->device.queues.present.handle = VK_NULL_HANDLE;

    if (context->device.handle) {
        LOG_INFO("Destroying logical device...");
        vkDestroyDevice(context->device.handle, context->allocator);
        context->device.handle = VK_NULL_HANDLE;
    }

    LOG_INFO("Releasing physical device...");
    context->device.physical_device = VK_NULL_HANDLE;

    DARRAY_DESTROY(context->device.swapchain_support.formats);
    DARRAY_DESTROY(context->device.swapchain_support.modes);
}

void vulkan_device_query_swapchain_support(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VulkanSwapchainSupportInfo* swapchainSupportInfo)
{
    VULKAN_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapchainSupportInfo->capabilities));

    u32 surfaceFormatCount = 0;
    VULKAN_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, NULL));
    if (surfaceFormatCount) {
        if (!swapchainSupportInfo->formats.capacity) {
            DARRAY_RESERVE(swapchainSupportInfo->formats, surfaceFormatCount, MEMORY_TAG_RENDERER);
        }

        VULKAN_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &surfaceFormatCount,
            swapchainSupportInfo->formats.data));
        swapchainSupportInfo->formats.size = surfaceFormatCount;
    }

    u32 presentModeCount = 0;
    VULKAN_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL));
    if (presentModeCount) {
        if (!swapchainSupportInfo->modes.capacity) {
            DARRAY_INIT(swapchainSupportInfo->modes, MEMORY_TAG_RENDERER);
            DARRAY_EXPAND(swapchainSupportInfo->modes, presentModeCount);
        }
        VULKAN_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &presentModeCount,
            swapchainSupportInfo->modes.data));
        swapchainSupportInfo->modes.size = presentModeCount;
    }
}

b8 vulkan_device_detect_depth_format(vulkan_device* device)
{
    VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags format_features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkFormat format;
    if (find_supported_format(
        device->physical_device, tiling,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        candidates, _countof(candidates), &format)) {
        device->depthFormat = format;
        return TRUE;
    }

    LOG_FATAL("vulkan_device_detect_depth_format: Failed to find a format for depth buffer");
    return FALSE;
}

b8 selectPhysicalDevice(vulkan_context* context)
{
    u32 deviceCount = 0;
    VULKAN_CHECK_RESULT(vkEnumeratePhysicalDevices(context->instance, &deviceCount, NULL));
    if (!deviceCount) {
        LOG_FATAL("No devices which support Vulkan were found");
        return FALSE;
    }
    DARRAY_DEFINE(VkPhysicalDevice, physicalDevices, deviceCount, MEMORY_TAG_RENDERER);
    VULKAN_CHECK_RESULT(vkEnumeratePhysicalDevices(context->instance, &deviceCount, physicalDevices.data));
    physicalDevices.size = deviceCount;

    for (u32 i = 0; i < physicalDevices.size; ++i) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(DARRAY_AT(physicalDevices, i), &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(DARRAY_AT(physicalDevices, i), &features);

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(DARRAY_AT(physicalDevices, i), &memoryProperties);

        PhysicalDeviceRequirements requirements = {};
        requirements.graphics = TRUE;
        requirements.present = TRUE;
        requirements.transfer = TRUE;
        // TODO: Enable this if compute will be required.
        // requirements.compute = TRUE;
        requirements.samplerAnisotropy = TRUE;
        requirements.discreteGpu = TRUE;
        DARRAY_INIT(requirements.deviceExtensionNames, MEMORY_TAG_STRING);
        DARRAY_PUSH(requirements.deviceExtensionNames, VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        PhysicalDeviceQueueFamilyInfo queueFamilyInfo = {};
        b8 result = physicalDeviceMeetsRequirements(
            DARRAY_AT(physicalDevices, i),
            context->surface,
            &properties,
            &features,
            &requirements,
            &queueFamilyInfo,
            &context->device.swapchain_support);
        if (result) {
            LOG_INFO("Selected device: '%s'.", properties.deviceName);
            switch (properties.deviceType) {
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    LOG_INFO("GPU type is Unknown.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    LOG_INFO("GPU type is Integrated.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    LOG_INFO("GPU type is Descrete.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    LOG_INFO("GPU type is Virtual.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    LOG_INFO("GPU type is CPU.");
                    break;
            }

            LOG_INFO(
                "GPU Driver version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion));

            LOG_INFO(
                "Vulkan API version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion));

            for (u32 j = 0; j < memoryProperties.memoryHeapCount; ++j) {
                u32 const GiB = 1024 * 1024 * 1024;
                f32 memorySize = (f32)memoryProperties.memoryHeaps[j].size / (f32)GiB;
                if (memoryProperties.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    LOG_INFO("Local GPU memory: %.2f GiB", memorySize);
                }
                else {
                    LOG_INFO("Shared system memory: %.2f GiB", memorySize);
                }
            }

            context->device.physical_device = DARRAY_AT(physicalDevices, i);
            context->device.queues.graphics.index = queueFamilyInfo.graphicsFamilyIndex;
            context->device.queues.present.index = queueFamilyInfo.presentFamilyIndex;
            context->device.queues.transfer.index = queueFamilyInfo.transferFamilyIndex;
            // TODO: set compute index here if needed.
            context->device.properties = properties;
            context->device.features = features;
            context->device.memoryProperties = memoryProperties;
            break;
        }
    }
    DARRAY_DESTROY(physicalDevices);

    if (!context->device.physical_device) {
        LOG_ERROR("No physical device found");
        return FALSE;
    }
    return TRUE;
}

b8 physicalDeviceMeetsRequirements(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkPhysicalDeviceProperties const* properties,
    VkPhysicalDeviceFeatures const* features,
    PhysicalDeviceRequirements const* requirements,
    PhysicalDeviceQueueFamilyInfo* queueFamilyInfo,
    VulkanSwapchainSupportInfo* swapchainSupportInfo)
{
    queueFamilyInfo->graphicsFamilyIndex = -1;
    queueFamilyInfo->presentFamilyIndex = -1;
    queueFamilyInfo->computeFamilyIndex = -1;
    queueFamilyInfo->transferFamilyIndex = -1;

    if (requirements->discreteGpu) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            LOG_FATAL("Device is not a discrete GPU, and one is required. Skipping the device");
            return FALSE;
        }
    }

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
    DARRAY_DEFINE(VkQueueFamilyProperties, queueFamilies, queueFamilyCount, MEMORY_TAG_RENDERER);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data);
    queueFamilies.size = queueFamilyCount;

    u8 minTransferScore = 255;
    for (u32 i = 0; i < queueFamilies.size; ++i) {
        u8 currentTransferScore = 0;
        if (DARRAY_AT(queueFamilies, i).queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyInfo->graphicsFamilyIndex = i;
            ++currentTransferScore;
        }

        if (DARRAY_AT(queueFamilies, i).queueFlags & VK_QUEUE_COMPUTE_BIT) {
            queueFamilyInfo->computeFamilyIndex = i;
            ++currentTransferScore;
        }

        if (DARRAY_AT(queueFamilies, i).queueFlags & VK_QUEUE_TRANSFER_BIT) {
            if (currentTransferScore <= minTransferScore) {
                minTransferScore = currentTransferScore;
                queueFamilyInfo->transferFamilyIndex = i;
            }
        }

        VkBool32 supportsPresent = VK_FALSE;
        VULKAN_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent));
        if (supportsPresent) {
            queueFamilyInfo->presentFamilyIndex = i;
        }
    }
    DARRAY_DESTROY(queueFamilies);

    if (
        (!requirements->graphics || (requirements->graphics && queueFamilyInfo->graphicsFamilyIndex != -1))
        && (!requirements->present || (requirements->present && queueFamilyInfo->presentFamilyIndex != -1))
        && (!requirements->compute || (requirements->compute && queueFamilyInfo->computeFamilyIndex != -1))
        && (!requirements->transfer || (requirements->transfer && queueFamilyInfo->transferFamilyIndex != -1))) {
        LOG_TRACE("Graphics queue family index %d", queueFamilyInfo->graphicsFamilyIndex);
        LOG_TRACE("Compute queue family index %d", queueFamilyInfo->computeFamilyIndex);
        LOG_TRACE("Transfer queue family index %d", queueFamilyInfo->transferFamilyIndex);
        LOG_TRACE("Present queue family index %d", queueFamilyInfo->presentFamilyIndex);

        vulkan_device_query_swapchain_support(physicalDevice, surface, swapchainSupportInfo);
        if (!swapchainSupportInfo->formats.size || !swapchainSupportInfo->modes.size) {
            LOG_INFO("Required swapchain support not present, skipping device");
            return FALSE;
        }

        if (requirements->deviceExtensionNames.size) {
            u32 propertyCount = 0;
            VULKAN_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &propertyCount, NULL));
            if (propertyCount) {
                DARRAY_DEFINE(VkExtensionProperties, availableExtensions, propertyCount, MEMORY_TAG_RENDERER);
                VULKAN_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(
                    physicalDevice,
                    NULL,
                    &propertyCount,
                    availableExtensions.data));
                availableExtensions.size = propertyCount;

                for (u32 i = 0; i < requirements->deviceExtensionNames.size; ++i) {
                    b8 found = FALSE;
                    for (u32 j = 0; j < availableExtensions.size; ++j) {
                        if (string_equal(
                                DARRAY_AT(requirements->deviceExtensionNames, i),
                                DARRAY_AT(availableExtensions, j).extensionName)) {
                            found = TRUE;
                            break;
                        }
                    }

                    if (!found) {
                        LOG_FATAL(
                            "Required device extension '%s' not found",
                            DARRAY_AT(requirements->deviceExtensionNames, i));
                        DARRAY_DESTROY(availableExtensions);
                        return FALSE;
                    }
                }
                DARRAY_DESTROY(availableExtensions);
            }
        }

        if (requirements->samplerAnisotropy && !features->samplerAnisotropy) {
            LOG_INFO("Device does not support samplerAnisotropy, skipping.");
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

b8 find_supported_format(
    VkPhysicalDevice physical_device,
    VkImageTiling tiling,
    VkFormatFeatureFlags format_features,
    VkFormat const* candidates,
    u32 candidate_count,
    VkFormat* format)
{
    for (u32 i = 0; i < candidate_count; ++i) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physical_device, candidates[i], &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & format_features == format_features)) {
            *format = candidates[i];
            return TRUE;
        }

        if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & format_features == format_features)) {
            *format = candidates[i];
            return TRUE;
        }
    }

    return FALSE;
}

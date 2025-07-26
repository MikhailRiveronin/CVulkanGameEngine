#include "VulkanSwapchain.h"
#include "Core/Logger.h"
#include "Core/Memory.h"
#include "VulkanDevice.h"

static b8 create(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain);
static void destroy(VulkanContext* context, VulkanSwapchain* swapchain);

b8 vulkanSwapchainCreate(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain)
{
    return create(context, width, height, swapchain);
}

b8 vulkanSwapchainRecreate(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain)
{
    destroy(context, swapchain);
    return create(context, width, height, swapchain);
}

void vulkanSwapchainDestroy(VulkanContext* context, VulkanSwapchain* swapchain)
{
    destroy(context, swapchain);
}

b8 vulkanSwapchainAcquireNextImageIndex(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    u64 timeoutNs,
    VkSemaphore imageAvailableSemaphore,
    VkFence fence,
    u32* imageIndex)
{
    VkResult result = vkAcquireNextImageKHR(
        context->device.logicalDevice,
        swapchain->handle,
        timeoutNs,
        imageAvailableSemaphore,
        fence,
        imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vulkanSwapchainRecreate(context, context->framebuffer.width, context->framebuffer.height, swapchain);
        return FALSE;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_FATAL("Failed to acquire swapchain image");
        return FALSE;
    }
    return TRUE;
}

void vulkanSwapchainPresent(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkSemaphore renderCompleteSemaphore,
    u32 presentImageIndex)
{
    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain->handle;
    presentInfo.pImageIndices = &presentImageIndex;
    presentInfo.pResults = NULL;

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        LOG_DEBUG("vkQueuePresentKHR returned VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR. Need swapchain recreation");
        vulkanSwapchainRecreate(context, context->framebuffer.width, context->framebuffer.height, swapchain);
    }
    else if (result != VK_SUCCESS) {
        LOG_FATAL("Failed to present swap chain image");
    }
}

b8 create(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain)
{
    VkExtent2D swapchainExtent = { width, height };
    swapchain->framesInFlight = 2;

    b8 found = FALSE;
    for (u32 i = 0; i < context->device.swapchainSupport.formats.size; ++i) {
        VkSurfaceFormatKHR format = DARRAY_AT(context->device.swapchainSupport.formats, i);
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain->surfaceFormat = format;
            found = TRUE;
            break;
        }
    }

    if (!found) {
        swapchain->surfaceFormat = DARRAY_AT(context->device.swapchainSupport.formats, 0);
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < context->device.swapchainSupport.modes.size; ++i) {
        VkPresentModeKHR mode = DARRAY_AT(context->device.swapchainSupport.modes, i);
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = mode;
            break;
        }
    }

    vulkanDeviceQuerySwapchainSupport(
        context->device.physicalDevice,
        context->surface,
        &context->device.swapchainSupport);

    if (context->device.swapchainSupport.capabilities.currentExtent.width != UINT32_MAX) {
        swapchainExtent = context->device.swapchainSupport.capabilities.currentExtent;
    }

    VkExtent2D min = context->device.swapchainSupport.capabilities.minImageExtent;
    VkExtent2D max = context->device.swapchainSupport.capabilities.maxImageExtent;
    swapchainExtent.width = CLAMP(swapchainExtent.width, min.width, max.width);
    swapchainExtent.height = CLAMP(swapchainExtent.height, min.height, max.height);

    u32 imageCount = context->device.swapchainSupport.capabilities.minImageCount + 1;
    imageCount = CLAMP(
        imageCount,
        context->device.swapchainSupport.capabilities.minImageCount,
        context->device.swapchainSupport.capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.surface = context->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = swapchain->surfaceFormat.format;
    createInfo.imageColorSpace = swapchain->surfaceFormat.colorSpace;
    createInfo.imageExtent = swapchainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (context->device.queues.graphics.index != context->device.queues.present.index) {
        u32 queueFamilyIndices[] = { context->device.queues.graphics.index, context->device.queues.present.index };
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }
    createInfo.preTransform = context->device.swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    VK_CHECK(vkCreateSwapchainKHR(context->device.logicalDevice, &createInfo, context->allocator, &swapchain->handle));

    context->currentFrame = 0;
    u32 swapchainImageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logicalDevice, swapchain->handle, &swapchainImageCount, NULL));
    if (!swapchain->images.capacity) {
        DARRAY_RESERVE(swapchain->images, swapchainImageCount, MEMORY_TAG_RENDERER);
        VK_CHECK(vkGetSwapchainImagesKHR(context->device.logicalDevice, swapchain->handle, &swapchainImageCount, swapchain->images.data));
        swapchain->images.size = swapchainImageCount;
    }

    DARRAY_RESERVE(swapchain->imageViews, swapchain->images.size, MEMORY_TAG_RENDERER);
    for (u8 i = 0; i < swapchain->images.size; ++i) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = NULL;
        createInfo.flags = 0;
        createInfo.image = DARRAY_AT(swapchain->images, i);
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchain->surfaceFormat.format;
        VkComponentMapping componentMapping = {};
        createInfo.components = componentMapping;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(
            context->device.logicalDevice,
            &createInfo,
            context->allocator,
            &DARRAY_AT(swapchain->imageViews, i)));
    }

    if (!vulkanDeviceDetectDepthFormat(&context->device)) {
        context->device.depthFormat = VK_FORMAT_UNDEFINED;
        LOG_FATAL("Failed to find a supported format");
    }

    vulkanImageCreate(
        context,
        VK_IMAGE_TYPE_2D,
        swapchainExtent.width,
        swapchainExtent.height,
        context->device.depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        TRUE,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &swapchain->depthBuffer);

    LOG_INFO("Swapchain created");
    return TRUE;
}

void destroy(VulkanContext* context, VulkanSwapchain* swapchain)
{
    vulkanImageDestroy(context, &swapchain->depthBuffer);
    for (u32 i = 0; i < swapchain->images.size; ++i) {
        vkDestroyImageView(context->device.logicalDevice, DARRAY_AT(swapchain->imageViews, i), context->allocator);
    }
    vkDestroySwapchainKHR(context->device.logicalDevice, swapchain->handle, context->allocator);
}

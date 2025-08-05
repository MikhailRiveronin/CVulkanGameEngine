#include "VulkanSwapchain.h"
#include "core/logger.h"
#include "Core/memory.h"
#include "VulkanDevice.h"

static b8 create(vulkan_context* context, u32 width, u32 height, VulkanSwapchain* swapchain);
static void destroy(vulkan_context* context, VulkanSwapchain* swapchain);

b8 vulkanSwapchainCreate(vulkan_context* context, u32 width, u32 height, VulkanSwapchain* swapchain)
{
    return create(context, width, height, swapchain);
}

b8 vulkan_swapchain_recreate(vulkan_context* context, u32 width, u32 height, VulkanSwapchain* swapchain)
{
    destroy(context, swapchain);
    return create(context, width, height, swapchain);
}

void vulkanSwapchainDestroy(vulkan_context* context, VulkanSwapchain* swapchain)
{
    destroy(context, swapchain);
}

b8 vulkan_swapchain_acquire_next_image(
    vulkan_context* context,
    VulkanSwapchain* swapchain,
    u64 timeout,
    VkSemaphore semaphore,
    VkFence fence,
    u32* imageIndex)
{
    VkResult result = vkAcquireNextImageKHR(
        context->device.handle, swapchain->handle,
        timeout, semaphore, fence,
        imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vulkan_swapchain_recreate(context, context->framebuffer_width, context->framebuffer_height, swapchain);
        return FALSE;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_FATAL("Failed to acquire swapchain image");
        return FALSE;
    }
    return TRUE;
}

void vulkan_swapchain_present(
    vulkan_context* context,
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
        vulkan_swapchain_recreate(context, context->framebuffer_width, context->framebuffer_height, swapchain);
    } else if (result != VK_SUCCESS) {
        LOG_FATAL("Failed to present swap chain image");
    }

    context->current_frame = (context->current_frame + 1) % context->swapchain.images.size;
}

b8 create(vulkan_context* context, u32 width, u32 height, VulkanSwapchain* swapchain)
{
    VkExtent2D swapchainExtent = { width, height };

    // Swapchain format
    b8 found = FALSE;
    for (u32 i = 0; i < context->device.swapchain_support.formats.size; ++i) {
        VkSurfaceFormatKHR format = context->device.swapchain_support.formats.data[i];
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            found = TRUE;
            swapchain->surfaceFormat = format;
            break;
        }
    }

    if (!found) {
        swapchain->surfaceFormat = context->device.swapchain_support.formats.data[0];
    }

    // Swapchain present mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < context->device.swapchain_support.modes.size; ++i) {
        VkPresentModeKHR mode = context->device.swapchain_support.modes.data[i];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = mode;
            break;
        }
    }

    // Swapchain extent
    vulkan_device_query_swapchain_support(
        context->device.physical_device,
        context->surface,
        &context->device.swapchain_support);

    if (context->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX) {
        swapchainExtent = context->device.swapchain_support.capabilities.currentExtent;
    }

    VkExtent2D min = context->device.swapchain_support.capabilities.minImageExtent;
    VkExtent2D max = context->device.swapchain_support.capabilities.maxImageExtent;
    swapchainExtent.width = CLAMP(swapchainExtent.width, min.width, max.width);
    swapchainExtent.height = CLAMP(swapchainExtent.height, min.height, max.height);
    context->framebuffer_width = swapchainExtent.width;
    context->framebuffer_height = swapchainExtent.height;

    u32 image_count = context->device.swapchain_support.capabilities.minImageCount + 1;
    if ((context->device.swapchain_support.capabilities.maxImageCount > 0)
        && (image_count > context->device.swapchain_support.capabilities.maxImageCount)) {
        image_count = context->device.swapchain_support.capabilities.maxImageCount;
    }
    swapchain->frames_in_flight = image_count - 1;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.surface = context->surface;
    createInfo.minImageCount = image_count;
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
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    createInfo.preTransform = context->device.swapchain_support.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    VK_CHECK(vkCreateSwapchainKHR(context->device.handle, &createInfo, context->allocator, &swapchain->handle));

    context->current_frame = 0;

    u32 swapchainImageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.handle, swapchain->handle, &swapchainImageCount, NULL));
    if (!swapchain->images.capacity) {
        DARRAY_RESERVE(swapchain->images, swapchainImageCount, MEMORY_TAG_RENDERER);
        swapchain->images.size = swapchainImageCount;
    }
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.handle, swapchain->handle, &swapchainImageCount, swapchain->images.data));

    DARRAY_RESERVE(swapchain->imageViews, swapchain->images.size, MEMORY_TAG_RENDERER);
    swapchain->imageViews.size = swapchain->images.size;
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
            context->device.handle,
            &createInfo,
            context->allocator,
            &DARRAY_AT(swapchain->imageViews, i)));
    }

    if (!vulkan_device_detect_depth_format(&context->device)) {
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

void destroy(vulkan_context* context, VulkanSwapchain* swapchain)
{
    vkDeviceWaitIdle(context->device.handle);
    vulkanImageDestroy(context, &swapchain->depthBuffer);
    for (u32 i = 0; i < swapchain->images.size; ++i) {
        vkDestroyImageView(context->device.handle, DARRAY_AT(swapchain->imageViews, i), context->allocator);
    }
    vkDestroySwapchainKHR(context->device.handle, swapchain->handle, context->allocator);
}

#include "VulkanFence.h"

void vulkanFenceCreate(vulkan_context* context, b8 createSignaled, VulkanFence* fence)
{
    fence->signaled = createSignaled;

    VkFenceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = createSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    VK_CHECK(vkCreateFence(context->device.handle, &createInfo, context->allocator, &fence->handle));
}

void vulkanFenceDestroy(vulkan_context* context, VulkanFence* fence)
{
    if (fence->handle) {
        vkDestroyFence(context->device.handle, fence->handle, context->allocator);
        fence->handle = VK_NULL_HANDLE;
    }
    fence->signaled = FALSE;
}

b8 vulkan_fence_wait(vulkan_context* context, VulkanFence* fence, u64 timeoutNs)
{
    if (!fence->signaled) {
        VkResult result = vkWaitForFences(context->device.handle, 1, &fence->handle, VK_TRUE, timeoutNs);
        switch (result) {
            case VK_SUCCESS:
                fence->signaled = TRUE;
                return TRUE;
            case VK_TIMEOUT:
                LOG_WARNING("vk_fence_wait - Timed out");
                break;
            case VK_ERROR_DEVICE_LOST:
                LOG_ERROR("vk_fence_wait - VK_ERROR_DEVICE_LOST.");
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                LOG_ERROR("vk_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY.");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                LOG_ERROR("vk_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY.");
                break;
            default:
                LOG_ERROR("vk_fence_wait - An unknown error has occurred.");
                break;
        }
    } else {
        return TRUE;
    }
    return FALSE;
}

void vulkan_fence_reset(vulkan_context* context, VulkanFence* fence)
{
    if (fence->signaled) {
        vkResetFences(context->device.handle, 1, &fence->handle);
        fence->signaled = FALSE;
    }
}

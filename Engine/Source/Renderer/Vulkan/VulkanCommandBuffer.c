#include "VulkanCommandBuffer.h"

void vulkanCommandBufferAllocate(
    vulkan_context* context,
    VkCommandPool pool,
    b8 primary,
    vulkan_command_buffer* commandBuffer)
{
    commandBuffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = NULL;
    allocateInfo.commandPool = pool;
    allocateInfo.level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(context->device.handle, &allocateInfo, &commandBuffer->handle));
    commandBuffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkanCommandBufferFree(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* commandBuffer)
{
    vkFreeCommandBuffers(
        context->device.handle,
        pool, 1, &commandBuffer->handle);

    commandBuffer->handle = VK_NULL_HANDLE;
    commandBuffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void vulkan_command_buffer_begin(
    vulkan_command_buffer* commandBuffer,
    b8 singleUse,
    b8 renderpassContinue,
    b8 simultaneousUse)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = NULL;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = NULL;
    if (singleUse) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (renderpassContinue) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (simultaneousUse) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VK_CHECK(vkBeginCommandBuffer(commandBuffer->handle, &beginInfo));
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkan_command_buffer_end(vulkan_command_buffer* commandBuffer)
{
    VK_CHECK(vkEndCommandBuffer(commandBuffer->handle));
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void vulkanCommandBufferUpdateSubmitted(vulkan_command_buffer* commandBuffer)
{
    commandBuffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vulkan_command_buffer_reset(vulkan_command_buffer* commandBuffer)
{
    commandBuffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkanCommandBufferAllocateAndBeginSingleUse(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* commandBuffer)
{
    vulkanCommandBufferAllocate(context, pool, TRUE, commandBuffer);
    vulkan_command_buffer_begin(commandBuffer, TRUE, FALSE, FALSE);
}

void vulkanCommandBufferEndSingleUse(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* commandBuffer,
    VkQueue queue)
{
    vulkan_command_buffer_end(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = NULL;
    submitInfo.pWaitDstStageMask = NULL;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = NULL;
    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

    VK_CHECK(vkQueueWaitIdle(queue));
    vulkanCommandBufferFree(context, pool, commandBuffer);
}

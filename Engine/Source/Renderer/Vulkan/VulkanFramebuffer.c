#include "VulkanFramebuffer.h"
#include "Core/Memory.h"

void vulkanFramebufferCreate(
    vulkan_context* context,
    vulkan_renderpass* renderpass,
    i32 width,
    i32 height,
    u32 attachmentCount,
    VkImageView* attachments,
    VulkanFramebuffer* framebuffer)
{
    framebuffer->renderpass = renderpass;
    framebuffer->attachmentCount = attachmentCount;
    framebuffer->attachments = memoryAllocate(attachmentCount * sizeof(*attachments), MEMORY_TAG_RENDERER);
    memoryCopy(framebuffer->attachments, attachments, attachmentCount * sizeof(*attachments));

    VkFramebufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.renderPass = framebuffer->renderpass->handle;
    createInfo.attachmentCount = framebuffer->attachmentCount;
    createInfo.pAttachments = framebuffer->attachments;
    createInfo.width = width;
    createInfo.height = height;
    createInfo.layers = 1;
    VK_CHECK(vkCreateFramebuffer(context->device.handle, &createInfo, context->allocator, &framebuffer->handle));
}

void vulkanFramebufferDestroy(vulkan_context* context, VulkanFramebuffer* framebuffer)
{
    vkDestroyFramebuffer(context->device.handle, framebuffer->handle, context->allocator);
    if (framebuffer->attachments) {
        memoryFree(
            framebuffer->attachments,
            framebuffer->attachmentCount * sizeof(*framebuffer->attachments),
            MEMORY_TAG_RENDERER);
    }
    framebuffer->handle = VK_NULL_HANDLE;
    framebuffer->attachmentCount = 0;
    framebuffer->renderpass = NULL;
}

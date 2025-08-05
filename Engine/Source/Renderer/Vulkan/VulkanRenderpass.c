#include "VulkanRenderpass.h"
#include "Core/memory.h"

void vulkan_renderpass_create(
    vulkan_context* context, 
    vulkan_renderpass* renderpass,
    f32 x, f32 y, f32 w, f32 h,
    f32 r, f32 g, f32 b, f32 a,
    f32 depth, u32 stencil)
{
    renderpass->depth = depth;
    renderpass->stencil = stencil;
    renderpass->x = x;
    renderpass->y = y;
    renderpass->width = w;
    renderpass->height = h;
    renderpass->r = r;
    renderpass->g = g;
    renderpass->b = b;
    renderpass->a = a;


    VkAttachmentDescription attachments[2];
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.flags = 0;
    colorAttachment.format = context->swapchain.surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.flags = 0;
    depthAttachment.format = context->device.depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachments[0] = colorAttachment;
    attachments[1] = depthAttachment;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentReference;
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depthAttachmentReference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpassDependency.dependencyFlags = 0;

    VkRenderPassCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.attachmentCount = _countof(attachments);
    createInfo.pAttachments = attachments;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &subpassDependency;
    VK_CHECK(vkCreateRenderPass(context->device.handle, &createInfo, context->allocator, &renderpass->handle));
}

void vulkanRenderpassDestroy(vulkan_context* context, vulkan_renderpass* renderpass)
{
    if (renderpass && renderpass->handle) {
        vkDestroyRenderPass(context->device.handle, renderpass->handle, context->allocator);
        renderpass->handle = VK_NULL_HANDLE;
    }
}

void vulkan_renderpass_begin(
    vulkan_command_buffer* commandBuffer,
    vulkan_renderpass* renderpass,
    VkFramebuffer framebuffer)
{
    VkClearValue clearValues[2];
    VkClearValue clearColor = {};
    clearColor.color.float32[0] = renderpass->r;
    clearColor.color.float32[1] = renderpass->g;
    clearColor.color.float32[2] = renderpass->b;
    clearColor.color.float32[3] = renderpass->a;
    VkClearValue clearDepth = {};
    clearDepth.depthStencil.depth = renderpass->depth;
    clearDepth.depthStencil.stencil = renderpass->stencil;
    clearValues[0] = clearColor;
    clearValues[1] = clearDepth;

    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = NULL;
    beginInfo.renderPass = renderpass->handle;
    beginInfo.framebuffer = framebuffer;
    beginInfo.renderArea.offset.x = renderpass->x;
    beginInfo.renderArea.offset.y = renderpass->y;
    beginInfo.renderArea.extent.width = renderpass->width;
    beginInfo.renderArea.extent.height = renderpass->height;
    beginInfo.clearValueCount = _countof(clearValues);
    beginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer->handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void vulkan_renderpass_end(vulkan_command_buffer* commandBuffer, vulkan_renderpass* renderpass)
{
    vkCmdEndRenderPass(commandBuffer->handle);
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

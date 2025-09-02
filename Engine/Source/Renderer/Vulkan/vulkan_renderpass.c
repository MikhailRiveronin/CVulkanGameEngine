#include "vulkan_renderpass.h"
#include "core/memory_utils.h"

void vulkan_renderpass_create(
    vulkan_context* context,
    vulkan_renderpass* renderpass,
    vec4 render_area,
    vec4 clear_color,
    f32 depth,
    u32 stencil,
    renderpass_clear_flag clear_flags,
    b8 has_prev_pass,
    b8 has_next_pass)
{
    renderpass->clear_flags = clear_flags;
    renderpass->render_area = render_area;
    renderpass->clear_colour = clear_color;
    renderpass->has_prev_pass = has_prev_pass;
    renderpass->has_next_pass = has_next_pass;

    renderpass->depth = depth;
    renderpass->stencil = stencil;

    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    u32 attachment_description_count = 0;
    VkAttachmentDescription attachments[2];

    b8 do_clear_color = renderpass->clear_flags & RENDERPASS_CLEAR_COLOUR_BUFFER_FLAG;
    VkAttachmentDescription color_attachment = {};
    color_attachment.flags = 0;
    color_attachment.format = context->swapchain.surfaceFormat.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = do_clear_color ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = has_prev_pass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = has_next_pass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[attachment_description_count++] = color_attachment;

    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment = {};
    VkAttachmentReference depth_attachment_reference = {};
    b8 do_clear_depth = renderpass->clear_flags & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG;
    if (do_clear_depth) {
        depth_attachment.flags = 0;
        depth_attachment.format = context->device.depthFormat;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = do_clear_depth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachments[1] = depth_attachment;

        depth_attachment_reference.attachment = 1;
        depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    } else {
        memory_zero(&attachments[attachment_description_count], sizeof(attachments[attachment_description_count]));
        subpass.pDepthStencilAttachment = 0;
    }

    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depth_attachment_reference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkSubpassDependency subpass_dependency = {};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpass_dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.attachmentCount = _countof(attachments);
    create_info.pAttachments = attachments;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies = &subpass_dependency;
    VK_CHECK(vkCreateRenderPass(context->device.handle, &create_info, context->allocator, &renderpass->handle));
}

void vulkan_renderpass_destroy(vulkan_context* context, vulkan_renderpass* renderpass)
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
    VkRenderPassBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.pNext = NULL;
    begin_info.renderPass = renderpass->handle;
    begin_info.framebuffer = framebuffer;
    begin_info.renderArea.offset.x = renderpass->render_area.x;
    begin_info.renderArea.offset.y = renderpass->render_area.y;
    begin_info.renderArea.extent.width = renderpass->render_area.z;
    begin_info.renderArea.extent.height = renderpass->render_area.w;

    begin_info.clearValueCount = 0;
    begin_info.pClearValues = 0;

    VkClearValue clear_values[2];
    memory_zero(clear_values, sizeof(*clear_values));
    b8 do_clear_colour = renderpass->clear_flags & RENDERPASS_CLEAR_COLOUR_BUFFER_FLAG;
    if (do_clear_colour) {
        memory_copy(clear_values[begin_info.clearValueCount].color.float32, renderpass->clear_colour.elements, sizeof(f32) * 4);
        begin_info.clearValueCount++;
    }

    b8 do_clear_depth = renderpass->clear_flags & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG;
    if (do_clear_depth) {
        memory_copy(clear_values[begin_info.clearValueCount].color.float32, renderpass->clear_colour.elements, sizeof(f32) * 4);
        clear_values[begin_info.clearValueCount].depthStencil.depth = renderpass->depth;

        b8 do_clear_stencil = (renderpass->clear_flags & RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG) != 0;
        clear_values[begin_info.clearValueCount].depthStencil.stencil = do_clear_stencil ? renderpass->stencil : 0;
        begin_info.clearValueCount++;
    }

    begin_info.clearValueCount = _countof(clear_values);
    begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(commandBuffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void vulkan_renderpass_end(vulkan_command_buffer* commandBuffer, vulkan_renderpass* renderpass)
{
    vkCmdEndRenderPass(commandBuffer->handle);
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

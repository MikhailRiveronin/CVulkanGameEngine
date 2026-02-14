#pragma once

#include "defines.h"

inline VkShaderModuleCreateInfo shader_module_create_info(size_t code_size, u32 const* code)
{
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.codeSize = code_size;
    create_info.pCode = code;
    return create_info;
}

inline VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule module)
{
    VkPipelineShaderStageCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.stage = stage;
    create_info.module = module;
    create_info.pName = "main";
    create_info.pSpecializationInfo = 0;
    return create_info;
}

inline VkVertexInputBindingDescription vertex_input_binding_description(u32 binding, u32 stride)
{
    VkVertexInputBindingDescription description = {};
    description.binding = binding;
    description.stride = stride;
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return description;
}

inline VkVertexInputAttributeDescription vertex_input_attribute_description(u32 location, u32 binding, VkFormat format, u32 offset)
{
    VkVertexInputAttributeDescription description = {};
    description.location = location;
    description.binding = binding;
    description.format = format;
    description.offset = offset;
    return description;
}

inline VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info(u32 binding_description_count, VkVertexInputBindingDescription const* vertex_binding_descriptions, u32 attribute_description_count VkVertexInputAttributeDescription const* vertex_attribute_descriptions)
{
    VkPipelineVertexInputStateCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.vertexBindingDescriptionCount = binding_description_count;
    create_info.pVertexBindingDescriptions = vertex_binding_descriptions;
    create_info.vertexAttributeDescriptionCount = attribute_description_count;
    create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions;
    return create_info;
}

inline VkDescriptorSetLayoutBinding descriptor_set_layout_binding(u32 binding, VkDescriptorType descriptor_type, u32 descriptor_count, VkShaderStageFlags stage_flags)
{
    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = binding;
    layout_binding.descriptorType = descriptor_type;
    layout_binding.descriptorCount = descriptor_count;
    layout_binding.stageFlags = stage_flags;
    layout_binding.pImmutableSamplers = 0;
    return layout_binding;
}

inline VkDescriptorSetLayoutBindingFlagsCreateInfo descriptor_set_layout_binding_flags_create_info(u32 binding_count, VkDescriptorBindingFlags const* binding_flags)
{
    VkDescriptorSetLayoutBindingFlagsCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    create_info.pNext = 0;
    create_info.bindingCount = binding_count;
    create_info.pBindingFlags = binding_flags;
    return create_info;
}

inline VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info(u32 binding_count, VkDescriptorSetLayoutBinding* bindings, void const* next)
{
    VkDescriptorSetLayoutCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.pNext = next;
    create_info.flags = 0;
    create_info.bindingCount = binding_count;
    create_info.pBindings = bindings;
    return create_info;
}

inline VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info()
{
    VkPipelineInputAssemblyStateCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    create_info.primitiveRestartEnable = VK_FALSE;
    return create_info;
}

inline VkPipelineTessellationStateCreateInfo pipeline_tessellation_state_create_info()
{
    VkPipelineTessellationStateCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.patchControlPoints = 0;
    return create_info;
}

inline VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info()
{
    VkPipelineRasterizationStateCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.depthClampEnable = VK_FALSE;
    create_info.rasterizerDiscardEnable = VK_FALSE;
    create_info.polygonMode = VK_POLYGON_MODE_FILL;
    create_info.cullMode = VK_CULL_MODE_NONE;
    create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    create_info.depthBiasEnable = VK_FALSE;
    create_info.depthBiasConstantFactor = 0.f;
    create_info.depthBiasClamp = 0.f;
    create_info.depthBiasSlopeFactor = 0.f;
    create_info.lineWidth = 1.f;
    return create_info;
}

inline VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info()
{
    VkPipelineMultisampleStateCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    create_info.sampleShadingEnable = VK_FALSE;
    create_info.minSampleShading = 0.f;
    create_info.pSampleMask = 0;
    create_info.alphaToCoverageEnable = VK_FALSE;
    create_info.alphaToOneEnable = VK_FALSE;
    return create_info;
}

inline VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info()
{
    VkPipelineDepthStencilStateCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.depthTestEnable = VK_TRUE;
    create_info.depthWriteEnable = VK_TRUE;
    create_info.depthCompareOp = VK_COMPARE_OP_LESS;
    create_info.depthBoundsTestEnable = VK_FALSE;
    create_info.stencilTestEnable = VK_FALSE;
    create_info.front = {};
    create_info.back = {};
    create_info.minDepthBounds = 0.f;
    create_info.maxDepthBounds = 1.f;
    return create_info;
}

inline VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state()
{
    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    return color_blend_attachment;
}

inline VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info(VkPipelineColorBlendAttachmentState color_blend_attachment_state)
{
    VkPipelineColorBlendStateCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.logicOpEnable = VK_FALSE;
    create_info.logicOp = VK_LOGIC_OP_CLEAR;
    create_info.attachmentCount = 1;
    create_info.pAttachments = &color_blend_attachment_state;
    create_info.blendConstants[0] = { 0.f };
    create_info.blendConstants[1] = { 0.f };
    create_info.blendConstants[2] = { 0.f };
    create_info.blendConstants[3] = { 0.f };
    return create_info;
}

VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info()
{
    VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.dynamicStateCount = _countof(dynamic_states);
    create_info.pDynamicStates = dynamic_states;
    return create_info;
}

VkPipelineLayoutCreateInfo pipeline_layout_create_info(u32 set_layout_count, VkDescriptorSetLayout const* set_layouts, u32 push_constant_range_count, VkPushConstantRange const* push_constant_ranges)
{
    VkPipelineLayoutCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.setLayoutCount = set_layout_count;
    create_info.pSetLayouts = set_layouts;
    create_info.pushConstantRangeCount = push_constant_range_count;
    create_info.pPushConstantRanges = push_constant_ranges;
    return create_info;
}

VkGraphicsPipelineCreateInfo graphics_pipeline_create_info(u32 stage_count, VkPipelineShaderStageCreateInfo const* stages, VkPipelineVertexInputStateCreateInfo const* vertex_input_state, VkPipelineInputAssemblyStateCreateInfo const* input_assembly_state, VkPipelineTessellationStateCreateInfo const* tessellation_state, VkPipelineViewportStateCreateInfo const* viewport_state, VkPipelineRasterizationStateCreateInfo const* rasterization_state, VkPipelineMultisampleStateCreateInfo const* multisample_state, VkPipelineDepthStencilStateCreateInfo const* depth_stencil_state, VkPipelineColorBlendStateCreateInfo const* color_blend_state, VkPipelineDynamicStateCreateInfo const* dynamic_state, VkPipelineLayout layout, VkRenderPass render_pass)
{
    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = 0;
    createInfo.flags = 0;
    createInfo.stageCount = stage_count;
    createInfo.pStages = stages;
    createInfo.pVertexInputState = vertex_input_state;
    createInfo.pInputAssemblyState = input_assembly_state;
    createInfo.pTessellationState = tessellation_state;
    createInfo.pViewportState = viewport_state;
    createInfo.pRasterizationState = rasterization_state;
    createInfo.pMultisampleState = multisample_state;
    createInfo.pDepthStencilState = depth_stencil_state;
    createInfo.pColorBlendState = color_blend_state;
    createInfo.pDynamicState = dynamic_state;
    createInfo.layout = layout;
    createInfo.renderPass = render_pass;
    createInfo.subpass = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = 0;
    return createInfo;
}

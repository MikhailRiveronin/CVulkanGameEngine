#include "vulkan_backend.h"

#include "VulkanPlatform.h"
#include "vulkan_device.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderpass.h"
#include "vulkan_command_buffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanFence.h"
#include "vulkan_utils.h"
#include "shaders/vulkan_material_shader.h"
#include "math/math_types.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"

#include "containers/darray.h"

#include "core/application.h"
#include "core/logger.h"
#include "core/string_utils.h"


static vulkan_context context;
static u32 cached_framebuffer_width;
static u32 cached_framebuffer_height;

#ifdef _DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    VkDebugUtilsMessengerCallbackDataEXT const* callbackData,
    void* userData);
#endif

static i32 findMemoryType(u32 memoryTypeBits, VkMemoryPropertyFlags properties);
static void createCommandBuffers(renderer_backend* backend);
static void regenerateFramebuffers(renderer_backend* backend, VulkanSwapchain* swapchain, vulkan_renderpass* renderpass);
static b8 recreate_swapchain(renderer_backend* backend);

static b8 create_buffers(vulkan_context* context);

b8 vulkan_backend_create(renderer_backend* backend, char const* app_name)
{
    context.allocator = NULL;
    context.findMemoryType = findMemoryType;
    applicationGetFramebufferSize(&cached_framebuffer_width, &cached_framebuffer_height);
    context.framebuffer_width = cached_framebuffer_width;
    context.framebuffer_height = cached_framebuffer_height;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = "Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    DARRAY_DEFINE(char const*, requiredLayers, 0, MEMORY_TAG_STRING);
#ifdef _DEBUG
    DARRAY_PUSH(requiredLayers, "VK_LAYER_KHRONOS_validation");

    LOG_DEBUG("Required layers:");
    for (u32 i = 0; i < requiredLayers.size; ++i) {
        LOG_DEBUG("    %s", DARRAY_AT(requiredLayers, i));
    }
#endif
    {
        u32 propertyCount;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&propertyCount, NULL));
        DARRAY_DEFINE(VkLayerProperties, availableLayers, propertyCount, MEMORY_TAG_RENDERER);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&propertyCount, availableLayers.data));
        availableLayers.size = propertyCount;

        for (u32 i = 0; i < requiredLayers.size; ++i) {
            b8 found = FALSE;
            for (u32 j = 0; j < availableLayers.size; ++j) {
                if (string_equal(DARRAY_AT(requiredLayers, i), DARRAY_AT(availableLayers, j).layerName)) {
                    found = TRUE;
                    break;
                }
            }

            if (!found) {
                LOG_FATAL("Required validation layer '%s' not found", DARRAY_AT(requiredLayers, i));
                return FALSE;
            }
        }
        DARRAY_DESTROY(availableLayers);
    }

    DARRAY_CSTRING requiredExtensions;
    DARRAY_INIT(requiredExtensions, MEMORY_TAG_STRING);
    DARRAY_PUSH(requiredExtensions, VK_KHR_SURFACE_EXTENSION_NAME);
    vulkan_platform_get_required_extensions(&requiredExtensions);
#ifdef _DEBUG
    DARRAY_PUSH(requiredExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    LOG_DEBUG("Required extensions:");
    for (u32 i = 0; i < requiredExtensions.size; ++i) {
        LOG_DEBUG("    %s", DARRAY_AT(requiredExtensions, i));
    }
#endif
    {
        u32 propertyCount;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, NULL));
        DARRAY_DEFINE(VkExtensionProperties, availableExtensions, propertyCount, MEMORY_TAG_RENDERER);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, availableExtensions.data));
        availableExtensions.size = propertyCount;

        for (u32 i = 0; i < requiredExtensions.size; ++i) {
            b8 found = FALSE;
            for (u32 j = 0; j < availableExtensions.size; ++j) {
                if (string_equal(DARRAY_AT(requiredExtensions, i), DARRAY_AT(availableExtensions, j).extensionName)) {
                    found = TRUE;
                    break;
                }
            }

            if (!found) {
                LOG_FATAL("Required extension '%s' not found", DARRAY_AT(requiredExtensions, i));
                return FALSE;
            }
        }
        DARRAY_DESTROY(availableExtensions);
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = requiredLayers.size;
    instanceCreateInfo.ppEnabledLayerNames = requiredLayers.data;
    instanceCreateInfo.enabledExtensionCount = requiredExtensions.size;
    instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data;
    VK_CHECK(vkCreateInstance(&instanceCreateInfo, context.allocator, &context.instance));
    LOG_INFO("Vulkan instance created");
    DARRAY_DESTROY(requiredLayers);
    DARRAY_DESTROY(requiredExtensions);

#ifdef _DEBUG
    VkDebugUtilsMessageSeverityFlagsEXT severity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    // severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    VkDebugUtilsMessageTypeFlagsEXT messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    // messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
    debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerCreateInfo.pNext = NULL;
    debugMessengerCreateInfo.flags = 0;
    debugMessengerCreateInfo.messageSeverity = severity;
    debugMessengerCreateInfo.messageType = messageType;
    debugMessengerCreateInfo.pfnUserCallback = debugUtilsMessengerCallback;
    debugMessengerCreateInfo.pUserData = NULL;

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    ASSERT_MSG(func, "Failed to load vkCreateDebugUtilsMessengerEXT");
    VK_CHECK(func(context.instance, &debugMessengerCreateInfo, context.allocator, &context.debugUtilsMessenger));
    LOG_INFO("Debug messenger created");
#endif

    if (!platform_create_vulkan_surface(&context)) {
        LOG_ERROR("Failed to create surface");
        return FALSE;
    }

    if (!vulkanDeviceCreate(&context)) {
        LOG_ERROR("Failed to create device");
        return FALSE;
    }

    if (!vulkanSwapchainCreate(&context, context.framebuffer_width, context.framebuffer_height, &context.swapchain)) {
        LOG_ERROR("Failed to create swapchain");
        return FALSE;
    }

    vulkan_renderpass_create(
        &context,
        &context.main_renderpass,
        0.0f, 0.0f, context.framebuffer_width, context.framebuffer_height,
        0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0);

    DARRAY_RESERVE(context.swapchain.framebuffers, context.swapchain.images.size, MEMORY_TAG_RENDERER);
    regenerateFramebuffers(backend, &context.swapchain, &context.main_renderpass);
    context.swapchain.framebuffers.size = context.swapchain.images.size;

    // Command buffer
    createCommandBuffers(backend);

    // Sync objects
    DARRAY_RESERVE(context.image_available_semaphors, context.swapchain.images.size, MEMORY_TAG_RENDERER);
    DARRAY_RESERVE(context.render_complete_semaphors, context.swapchain.images.size, MEMORY_TAG_RENDERER);
    DARRAY_RESERVE(context.fences_in_flight, context.swapchain.images.size, MEMORY_TAG_RENDERER);
    DARRAY_RESERVE(context.imagesInFlight, context.swapchain.images.size, MEMORY_TAG_RENDERER);

    for (u32 i = 0; i < context.image_available_semaphors.capacity; ++i) {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = NULL;
        semaphoreCreateInfo.flags = 0;
        VK_CHECK(vkCreateSemaphore(
            context.device.handle,
            &semaphoreCreateInfo,
            context.allocator,
            &DARRAY_AT(context.image_available_semaphors, i)));
        VK_CHECK(vkCreateSemaphore(
            context.device.handle,
            &semaphoreCreateInfo,
            context.allocator,
            &DARRAY_AT(context.render_complete_semaphors, i)));

        vulkanFenceCreate(&context, TRUE, &DARRAY_AT(context.fences_in_flight, i));
    }

    // Create builtin shaders
    if (!vulkan_material_shader_create(&context, backend->default_diffuse, &context.material_shader)) {
        LOG_ERROR("Error loading built-in basic_lighting shader.");
        return FALSE;
    }

    create_buffers(&context);

    // TODO: temp code
    vertex_3d vertices[3];
    vertices[0].position.x = 0.0;
    vertices[0].position.y = -0.5;
    vertices[0].position.z = 0.0;
    vertices[1].position.x = 0.5;
    vertices[1].position.y = 0.5;
    vertices[1].position.z = 0.0;
    vertices[2].position.x = 0.0;
    vertices[2].position.y = 0.5;
    vertices[2].position.z = 0.0;

    u32 indices[3] = { 0, 1, 2 };

    vulkan_buffer_upload_data(
        &context,
        context.device.graphics_command_pool,
        0, context.device.queues.graphics.handle,
        &context.object_vertex_buffer,
        0, sizeof(*vertices) * _countof(vertices),
        vertices);

    vulkan_buffer_upload_data(
        &context,
        context.device.graphics_command_pool,
        0, context.device.queues.graphics.handle,
        &context.object_index_buffer,
        0, sizeof(*indices) * _countof(indices),
        indices);

    u32 object_id;
    if (!vulkan_material_shader_acquire_resources(&context, &context.material_shader, &object_id)) {
        LOG_ERROR("Failed to acquire shader resources");
        return FALSE;
    }

    // TODO: temp code

    LOG_INFO("Vulkan renderer initialized");
    return TRUE;
}

void vulkan_backend_destroy(struct renderer_backend* backend)
{
    vkDeviceWaitIdle(context.device.handle);

    // Vertex/index buffers
    vulkan_buffer_destroy(&context, &context.object_vertex_buffer);
    vulkan_buffer_destroy(&context, &context.object_index_buffer);

    // Shader objects
    vulkan_material_shader_destroy(&context, &context.material_shader);

    // Sync objects
    for (u32 i = 0; i < context.image_available_semaphors.capacity; ++i) {
        vkDestroySemaphore(context.device.handle, DARRAY_AT(context.image_available_semaphors, i), context.allocator);
        vkDestroySemaphore(context.device.handle, DARRAY_AT(context.render_complete_semaphors, i), context.allocator);
        vulkanFenceDestroy(&context, &DARRAY_AT(context.fences_in_flight, i));

        context.image_available_semaphors.data[i] = VK_NULL_HANDLE;
        context.render_complete_semaphors.data[i] = VK_NULL_HANDLE;
    }
    DARRAY_DESTROY(context.image_available_semaphors);
    DARRAY_DESTROY(context.render_complete_semaphors);
    DARRAY_DESTROY(context.fences_in_flight);
    DARRAY_DESTROY(context.imagesInFlight);

    for (u8 i = 0; i < context.command_buffers.size; ++i) {
        vulkanCommandBufferFree(
            &context,
            context.device.graphics_command_pool,
            &DARRAY_AT(context.command_buffers, i));
    }
    DARRAY_DESTROY(context.command_buffers);

    // Framebuffers
    for (u32 i = 0; i < context.swapchain.framebuffers.size; ++i) {
        vulkanFramebufferDestroy(&context, &DARRAY_AT(context.swapchain.framebuffers, i));
    }

    // Renderpass
    vulkanRenderpassDestroy(&context, &context.main_renderpass);

    // Swapchain
    vulkanSwapchainDestroy(&context, &context.swapchain);
    vulkanDeviceDestroy(&context);
    vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
#ifdef _DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context.instance,
            "vkDestroyDebugUtilsMessengerEXT");
    ASSERT_MSG(func, "Failed to load vkDestroyDebugUtilsMessengerEXT");
    func(context.instance, context.debugUtilsMessenger, context.allocator);
#endif
    vkDestroyInstance(context.instance, context.allocator);
}

b8 vulkan_backend_begin_frame(renderer_backend* backend, f64 delta_time)
{
    context.frame_delta_time = delta_time;

    VkDevice device = context.device.handle;
    vulkan_command_buffer* command_buffer = &context.command_buffers.data[context.current_frame];

    // Return if recreating swapchain
    if (context.recreating_swapchain) {
        VkResult result = vkDeviceWaitIdle(device);
        if (!vulkan_result_is_success(result)) {
            LOG_ERROR("vulkan_backend_begin_frame: vkDeviceWaitIdle(1) failed: '%s'",
                vulkan_result_string(result, TRUE));
            return FALSE;
        }

        LOG_INFO("Recreating swapchain");
        return FALSE;
    }

    // If framebuffer has been resized, recreate swapchain
    if (context.framebuffer_generation != context.framebuffer_last_generation) {
        VkResult result = vkDeviceWaitIdle(device);
        if (!vulkan_result_is_success(result)) {
            LOG_ERROR("vulkan_backend_begin_frame: vkDeviceWaitIdle(2) failed: '%s'",
                vulkan_result_string(result, TRUE));
            return FALSE;
        }

        // If the swapchain recreation failed (because, for example, the window was minimized),
        // don't unset recreating_swapchain
        if (!recreate_swapchain(backend)) {
            return FALSE;
        }

        // Swapchain resized but render on the next frame
        LOG_INFO("Swapchain resized");
        return FALSE;
    }

    if (!vulkan_fence_wait(&context, &context.fences_in_flight.data[context.current_frame], UINT64_MAX)) {
        LOG_WARNING("Failed to wait for the current frame to complete");
        return FALSE;
    }

    if (!vulkan_swapchain_acquire_next_image(
        &context, &context.swapchain, UINT64_MAX,
        context.image_available_semaphors.data[context.current_frame],
        VK_NULL_HANDLE, &context.image_index)) {
        return FALSE;
    }

    vulkan_command_buffer_reset(command_buffer);
    vulkan_command_buffer_begin(command_buffer, FALSE, FALSE, FALSE);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)context.framebuffer_width;
    viewport.height = (f32)context.framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = context.framebuffer_width;
    scissor.extent.height = context.framebuffer_height;

    vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

    context.main_renderpass.width = context.framebuffer_width;
    context.main_renderpass.height = context.framebuffer_height;

    vulkan_renderpass_begin(
        command_buffer, &context.main_renderpass,
        context.swapchain.framebuffers.data[context.image_index].handle);

    

    return TRUE;
}

void vulkan_backend_update_global_state(mat4 view, mat4 proj, vec3 view_pos, vec4 ambient_color, i32 mode)
{
    vulkan_command_buffer* command_buffer = &context.command_buffers.data[context.current_frame];

    vulkan_material_shader_use(&context, &context.material_shader);

    context.material_shader.global_data.view = view;
    context.material_shader.global_data.proj = proj;

    vulkan_material_shader_update_global_state(&context, &context.material_shader, context.frame_delta_time);
}

b8 vulkan_backend_end_frame(struct renderer_backend* backend, f64 deltaTime)
{
    vulkan_command_buffer* command_buffer = &context.command_buffers.data[context.current_frame];

    vulkan_renderpass_end(command_buffer, &context.main_renderpass);
    vulkan_command_buffer_end(command_buffer);

    vulkan_fence_reset(&context, &context.fences_in_flight.data[context.current_frame]);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &context.image_available_semaphors.data[context.current_frame];

    VkPipelineStageFlags wait_dst_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.pWaitDstStageMask = wait_dst_stages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer->handle;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &context.render_complete_semaphors.data[context.current_frame];
    VK_CHECK(vkQueueSubmit(
        context.device.queues.graphics.handle, 1, &submitInfo,
        context.fences_in_flight.data[context.current_frame].handle));

    vulkanCommandBufferUpdateSubmitted(command_buffer);

    vulkan_swapchain_present(
        &context, &context.swapchain,
        context.device.queues.graphics.handle, context.device.queues.present.handle,
        context.render_complete_semaphors.data[context.current_frame], context.image_index);

    return TRUE;
}

void vulkan_backend_on_resize(renderer_backend* backend, i16 width, i16 height)
{
    cached_framebuffer_width = width;
    cached_framebuffer_height = height;
    context.framebuffer_generation++;

    LOG_INFO("vulkan_backend_on_resize: width %i, height %i, framebuffer generation %llu",
        width, height, context.framebuffer_generation);
}

void vulkan_backend_update_object_state(geometry_render_data render_data)
{
    vulkan_command_buffer* command_buffer = &context.command_buffers.data[context.current_frame];

    vulkan_material_shader_update_object_state(&context, &context.material_shader, render_data);

    // TODO: temp code
    vulkan_material_shader_use(&context, &context.material_shader);

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(command_buffer->handle, 0, 1, &context.object_vertex_buffer.handle, offsets);
    vkCmdBindIndexBuffer(command_buffer->handle, context.object_index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(command_buffer->handle, 3, 1, 0, 0, 0);
}

void vulkan_backend_create_texture(
    char const* name,
    b8 auto_release,
    i32 width,
    i32 height,
    i32 channel_count,
    u8 const* pixels,
    b8 has_transparency,
    texture* texture)
{
    texture->widht = width;
    texture->height = height;
    texture->channel_count = channel_count;
    texture->generation = INVALID_ID;

    texture->internal_data = memory_allocate(sizeof(vulkan_texture_data), MEMORY_TAG_TEXTURE);
    vulkan_texture_data* data = texture->internal_data;

    VkDeviceSize image_size = width * height * channel_count;

    VkFormat image_format = VK_FORMAT_R8G8B8_SNORM;

    // Staging buffer
    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlagBits memory_properties =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan_buffer staging;
    vulkan_buffer_create(&context, image_size, usage, memory_properties, TRUE, &staging);

    vulkan_buffer_load_data(&context, &staging, 0, image_size, 0, pixels);

    vulkan_image_create(
        &context,
        VK_IMAGE_TYPE_2D,
        width, height,
        image_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        TRUE,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &data->image);

    VkCommandPool pool = context.device.graphics_command_pool;
    VkQueue queue = context.device.queues.graphics.handle;
    vulkan_command_buffer temp_command_buffer;
    vulkan_command_buffer_allocate_and_begin_single_use(&context, pool, &temp_command_buffer);

    vulkan_image_transition_layout(
        &context,
        &temp_command_buffer,
        &data->image,
        image_format,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vulkan_image_copy_from_buffer(&context, &temp_command_buffer, &data->image, staging.handle);
    vulkan_buffer_destroy(&context, &staging);

    vulkan_image_transition_layout(
        &context,
        &temp_command_buffer,
        &data->image,
        image_format,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vulkan_command_buffer_end_single_use(&context, pool, &temp_command_buffer, queue);

    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.pNext = 0;
    sampler_create_info.flags = 0;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.mipLodBias = 0.f;
    sampler_create_info.anisotropyEnable = VK_TRUE;
    sampler_create_info.maxAnisotropy = 1.f;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_NEVER;
    sampler_create_info.minLod = 0.f;
    sampler_create_info.maxLod = 0.f;
    sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    VK_CHECK(vkCreateSampler(context.device.handle, &sampler_create_info, context.allocator, &data->sampler));

    texture->has_transparency = has_transparency;
    texture->generation++;
}

void vulkan_backend_destroy_texture(texture* texture)
{
    vkDeviceWaitIdle(context.device.handle);

    vulkan_texture_data* data = texture->internal_data;

    vulkan_image_destroy(&context, &data->image);
    memory_zero(&data->image, sizeof(data->image));
    vkDestroySampler(context.device.handle, data->sampler, context.allocator);
    data->sampler = 0;

    memory_free(texture->internal_data, sizeof(vulkan_texture_data), MEMORY_TAG_TEXTURE);
    memory_zero(texture, sizeof(*texture));
}

#ifdef _DEBUG
VkBool32 debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    VkDebugUtilsMessengerCallbackDataEXT const* callbackData,
    void* userData)
{
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_ERROR("%s", callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARNING("%s", callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_INFO("%s", callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG_TRACE("%s", callbackData->pMessage);
            break;
    }
    return VK_FALSE;
}
#endif

i32 findMemoryType(u32 memoryTypeBits, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memoryProperties);
    for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((memoryTypeBits & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
            return i;
        }
    }
    return -1;
}

void createCommandBuffers(renderer_backend* backend)
{
    if (!context.command_buffers.capacity) {
        DARRAY_RESERVE(context.command_buffers, context.swapchain.images.size, MEMORY_TAG_RENDERER);
    }

    for (u8 i = 0; i < context.swapchain.images.size; ++i) {
        if (DARRAY_AT(context.command_buffers, i).handle) {
            vulkanCommandBufferFree(
                &context, context.device.graphics_command_pool,
                &DARRAY_AT(context.command_buffers, i));
        }
        vulkanCommandBufferAllocate(
            &context, context.device.graphics_command_pool,
            TRUE, &DARRAY_AT(context.command_buffers, i));
    }
    LOG_DEBUG("Vulkan command buffers created");
}

void regenerateFramebuffers(renderer_backend* backend, VulkanSwapchain* swapchain, vulkan_renderpass* renderpass)
{
    for (u8 i = 0; i < swapchain->images.size; ++i) {
        VkImageView attachments[] = { DARRAY_AT(swapchain->imageViews, i), swapchain->depthBuffer.view };
        vulkanFramebufferCreate(
            &context, renderpass,
            context.framebuffer_width,
            context.framebuffer_height,
            _countof(attachments), attachments,
            &DARRAY_AT(context.swapchain.framebuffers, i));
    }
}

b8 recreate_swapchain(renderer_backend* backend)
{
    // If already being recreated, do not try again
    if (context.recreating_swapchain) {
        LOG_DEBUG("Swapchain is already being recreated");
        return FALSE;
    }

    if ((context.framebuffer_width) == 0 || (context.framebuffer_height == 0)) {
        LOG_DEBUG("Recreating swapchain when window is minimized");
        return FALSE;
    }

    context.recreating_swapchain = TRUE;
    vkDeviceWaitIdle(context.device.handle);

    vulkan_device_query_swapchain_support(
        context.device.physical_device,
        context.surface,
        &context.device.swapchain_support);
    vulkan_device_detect_depth_format(&context.device);

    vulkan_swapchain_recreate(&context, cached_framebuffer_width, cached_framebuffer_height, &context.swapchain);

    // Sync the framebuffer size with the cached sizes.
    context.framebuffer_width = cached_framebuffer_width;
    context.framebuffer_height = cached_framebuffer_height;
    context.main_renderpass.width = context.framebuffer_width;
    context.main_renderpass.height = context.framebuffer_height;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;
    context.framebuffer_last_generation = context.framebuffer_generation;

    // cleanup swapchain
    for (u32 i = 0; i < context.swapchain.images.size; ++i) {
        vulkanCommandBufferFree(&context, context.device.graphics_command_pool, &context.command_buffers.data[i]);
    }

    // Framebuffers.
    for (u32 i = 0; i < context.swapchain.images.size; ++i) {
        vulkanFramebufferDestroy(&context, &context.swapchain.framebuffers.data[i]);
    }

    context.main_renderpass.x = 0;
    context.main_renderpass.y = 0;
    context.main_renderpass.width = context.framebuffer_width;
    context.main_renderpass.height = context.framebuffer_height;

    regenerateFramebuffers(backend, &context.swapchain, &context.main_renderpass);

    createCommandBuffers(backend);

    // Clear the recreating flag.
    context.recreating_swapchain = FALSE;

    return TRUE;
}

b8 create_buffers(vulkan_context* context)
{
    VkMemoryPropertyFlagBits memory_property_flag_bits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    u64 vertex_buffer_size = 1024 * 1024 * sizeof(vertex_3d);
    if (!vulkan_buffer_create(
        context,
        vertex_buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        memory_property_flag_bits,
        TRUE,
        &context->object_vertex_buffer)) {
        LOG_ERROR("Failed to create vertex buffer");
        return FALSE;
    }

    context->geometry_vertex_offset = 0;

    u64 index_buffer_size = 1024 * 1024 * sizeof(u32);
    if (!vulkan_buffer_create(
        context,
        index_buffer_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        memory_property_flag_bits,
        TRUE,
        &context->object_index_buffer)) {
        LOG_ERROR("Failed to create index buffer");
        return FALSE;
    }

    context->geometry_index_offset = 0;

    return TRUE;
}

#include "application.h"

#include "clock.h"
#include "config.h"
#include "event_system.h"
#include "input.h"
#include "logger.h"
#include "math/math_types.h"
#include "memory/linear_allocator.h"
#include "platform/platform.h"
#include "renderer/renderer_frontend.h"
#include "systems/geometry_system.h"
#include "systems/resource_system.h"
#include "systems/material_system.h"
#include "systems/memory_system.h"
#include "systems/texture_system.h"



#include "string_utils.h"

typedef struct application_state
{
    game_instance* instance;
    b8 running;
    b8 suspended;
    linear_allocator systems_allocator;

    struct
    {
        u64 required_memory;
        void* block;
    } event_system;

    struct
    {
        u64 required_memory;
        void* block;
    } logger_system;

    struct
    {
        u64 required_memory;
        void* block;
    } input_system;

    struct
    {
        u64 required_memory;
        void* block;
    } platform_system;

    struct
    {
        u64 required_memory;
        void* block;
    } resource_system;





    i32 width;
    i32 height;
    clock clock;
    f64 lastTime;
    platform_state platform;





    struct
    {
        u64 required_memory;
        void* block;
    } renderer_system;

    struct
    {
        u64 required_memory;
        void* block;
    } texture_system;

    struct
    {
        u64 required_memory;
        void* block;
    } material_system;

    struct
    {
        u64 required_memory;
        void* block;
    } geometry_system;

    // TODO: temp
    Geometry* test_geometry;
    Geometry* test_ui_geometry;
    // TODO: temp
} application_state;

static application_state* state;

b8 application_on_event(u16 code, void const* sender, void const* listener, event_context context);
b8 application_on_key(u16 code, void const* sender, void const* listener, event_context context);
b8 application_on_resize(u16 code, void const* sender, void const* listener, event_context context);

b8 application_init(game_instance* instance)
{
    if (instance->application_block)
    {
        KERROR("application_init: Was already called");
        return FALSE;
    }

    memory_system_configuration memory_system_config = {};
    memory_system_config.tracked_memory = GIBIBYTES(1);
    if (!memory_system_startup(memory_system_config))
    {
        LOG_ERROR("application_init: Failed to initialize memory system");
        return FALSE;
    }

    instance->internal = memory_system_allocate(instance->required_memory, MEMORY_TAG_GAME);
    instance->application_block = memory_system_allocate(sizeof(*state), MEMORY_TAG_APPLICATION);
    state = instance->application_block;
    state->instance = instance;
    state->running = FALSE;
    state->suspended = FALSE;

    u64 systems_allocator_requered_memory = MEBIBYTES(64);
    linear_allocator_create(systems_allocator_requered_memory, 0, &state->systems_allocator);

    event_system_startup(&state->event_system.required_memory, 0);
    state->event_system.block = linear_allocator_allocate(&state->systems_allocator, state->event_system.required_memory);
    if (!event_system_startup(&state->event_system.required_memory, state->event_system.block))
    {
        LOG_FATAL("application_init: Failed to startup event system");
        return FALSE;
    }

    logger_system_startup(&state->logger_system.required_memory, 0);
    state->logger_system.block = linear_allocator_allocate(&state->systems_allocator, state->logger_system.required_memory);
    if (!logger_system_startup(&state->logger_system.required_memory, state->logger_system.block))
    {
        LOG_FATAL("application_init: Failed to startup logger system");
        return FALSE;
    }

    input_system_startup(&state->input_system.required_memory, 0);
    state->input_system.block = linear_allocator_allocate(&state->systems_allocator, state->input_system.required_memory);
    if (!input_system_startup(&state->input_system.required_memory, state->input_system.block))
    {
        LOG_FATAL("application_init: Failed to startup input system");
        return FALSE;
    }

    platform_system_startup(&state->platform_system.required_memory, 0, 0, 0, 0, 0, 0, 0);
    state->platform_system.block = linear_allocator_allocate(&state->systems_allocator, state->platform_system.required_memory);
    if (!platform_system_startup(&state->platform_system.required_memory, state->platform_system.block, &state->platform, state->instance->application_config.name, state->instance->application_config.x, state->instance->application_config.y, state->instance->application_config.width, state->instance->application_config.height))
    {
        LOG_FATAL("application_init: Failed to startup platform system");
        return FALSE;
    }

    Resource_System_Config resource_system_config;
    resource_system_config.asset_folder_path = ASSETS_DIR;
    resource_system_config.max_loader_count = 32;
    resource_system_startup(&state->resource_system.required_memory, 0, resource_system_config);
    state->resource_system.block = linear_allocator_allocate(&state->systems_allocator, state->resource_system.required_memory);
    if(!resource_system_startup(&state->resource_system.required_memory, state->resource_system.block, resource_system_config))
    {
        LOG_FATAL("application_init: Failed to startup resource system");
        return FALSE;
    }

    renderer_system_startup(&state->renderer_system.required_memory, 0, 0);
    state->renderer_system.block = linear_allocator_allocate(&state->systems_allocator, state->renderer_system.required_memory);
    if (!renderer_system_startup(&state->renderer_system.required_memory, state->renderer_system.block, instance->application_config.name))
    {
        LOG_FATAL("application_init: Failed to initialize renderer system");
        return FALSE;
    }

    Texture_System_Configuration texture_system_config;
    texture_system_config.max_texture_count = 65536;
    texture_system_startup(&state->texture_system.required_memory, 0, texture_system_config);
    state->texture_system.block = linear_allocator_allocate(&state->systems_allocator, state->texture_system.required_memory);
    if (!texture_system_startup(&state->texture_system.required_memory, state->texture_system.block, texture_system_config))
    {
        LOG_FATAL("application_init: Failed to initialize texture system. Shutting down...");
        return FALSE;
    }

    Material_System_Config material_sys_config;
    material_sys_config.max_material_count = 4096;
    material_system_startup(&state->material_system.required_memory, 0, material_sys_config);
    state->material_system.block = linear_allocator_allocate(&state->systems_allocator, state->material_system.required_memory);
    if (!material_system_startup(&state->material_system.required_memory, state->material_system.block, material_sys_config)) {
        LOG_FATAL("application_init: Failed to initialize material_resource system");
        return FALSE;
    }

    Geometry_System_Config geometry_system_config;
    geometry_system_config.max_geometry_count = 4096;
    geometry_system_startup(&state->geometry_system.required_memory, 0, geometry_system_config);
    state->geometry_system.block = linear_allocator_allocate(&state->systems_allocator, state->material_system.required_memory);
    if (!geometry_system_startup(&state->geometry_system.required_memory, state->geometry_system.block, geometry_system_config))
    {
        LOG_FATAL("application_init: Failed to initialize geometry system");
        return FALSE;
    }

    // TODO: Temp
    Geometry_Config g_config = geometry_system_generate_plane_config(15.0f, 5.0f, 5, 5, 5.0f, 2.0f, "test geometry", "test_material");
    state->test_geometry = geometry_system_acquire_from_config(g_config, TRUE);

    memory_system_free(g_config.vertices, sizeof(vertex_3d) * g_config.vertex_count, MEMORY_TAG_ARRAY);
    memory_system_free(g_config.indices, sizeof(u32) * g_config.index_count, MEMORY_TAG_ARRAY);

    // Load up some test UI geometry.
    Geometry_Config ui_config;
    ui_config.vertex_size = sizeof(vertex_2d);
    ui_config.vertex_count = 4;
    ui_config.index_size = sizeof(u32);
    ui_config.index_count = 6;
    string_copy(ui_config.material_name, "test_ui_material");
    string_copy(ui_config.name, "test_ui_geometry");

    const f32 f = 512.0f;
    vertex_2d uiverts[4];
    uiverts[0].pos.x = 0.0f;  // 0    3
    uiverts[0].pos.y = 0.0f;  //
    uiverts[0].tex_coord.x = 0.0f;  //
    uiverts[0].tex_coord.y = 0.0f;  // 2    1

    uiverts[1].pos.y = f;
    uiverts[1].pos.x = f;
    uiverts[1].tex_coord.x = 1.0f;
    uiverts[1].tex_coord.y = 1.0f;

    uiverts[2].pos.x = 0.0f;
    uiverts[2].pos.y = f;
    uiverts[2].tex_coord.x = 0.0f;
    uiverts[2].tex_coord.y = 1.0f;

    uiverts[3].pos.x = f;
    uiverts[3].pos.y = 0.0;
    uiverts[3].tex_coord.x = 1.0f;
    uiverts[3].tex_coord.y = 0.0f;
    ui_config.vertices = uiverts;

    // Indices - counter-clockwise
    u32 uiindices[6] = {2, 1, 0, 3, 0, 1};
    ui_config.indices = uiindices;

    // Get UI geometry from config.
    state->test_ui_geometry = geometry_system_acquire_from_config(ui_config, true);
    // TODO: Temp

    if (!state->instance->init(state->instance))
    {
        LOG_FATAL("application_init: Failed to initialize game");
        return FALSE;
    }

    state->width = instance->application_config.width;
    state->height = instance->application_config.height;

    event_register(EVENT_CODE_APPLICATION_QUIT, NULL, application_on_event);
    event_register(EVENT_CODE_KEY_PRESSED, NULL, application_on_key);
    event_register(EVENT_CODE_RESIZE, NULL, application_on_resize);

    // TODO: What for?
    state->suspended = FALSE;
    state->instance->on_resize(state->instance, state->width, state->height);

    return TRUE;
}

b8 application_run(void)
{
    state->running = TRUE;
    memory_system_print_usage();

    clock_start(&state->clock);
    clock_update(&state->clock);
    state->lastTime = state->clock.elapsed;
    f64 runningTime = 0.0;
    u8 frameCount = 0;
    const f64 targetFrameRate = 1.0 / 60.0;

    while (state->running) {
        if (!platformProcMessages(&state->platform)) {
            state->running = FALSE;
            break;
        }

        if (!state->running) {
            break;
        }

        if (!state->suspended) {
            clock_update(&state->clock);
            f64 currentTime = state->clock.elapsed;
            f64 delta_time = currentTime - state->lastTime;
            f64 frameStartTime = platform_get_absolute_time();


            if (!state->instance->on_update(state->instance, delta_time)) {
                LOG_FATAL("Game update failed");
                state->running = FALSE;
                break;
            }

            if (!state->instance->on_render(state->instance, delta_time)) {
                LOG_FATAL("Game render failed");
                state->running = FALSE;
                break;
            }

            // TODO: temporary solution
            render_packet packet;
            packet.delta_time = delta_time;

            // TODO: temp
            geometry_render_data test_render;
            test_render.geometry = state->test_geometry;
            glm_mat4_identity(&test_render.world[0]);

            packet.geometry_count = 1;
            packet.render_data = &test_render;
            // TODO: end temp

            packet.ui_geometry_count = 0;
            packet.ui_render_data = 0;

            renderer_frontend_draw_frame(&packet);

            f64 frameEndTime = platform_get_absolute_time();
            f64 frameElapsedTime = frameEndTime - frameStartTime;
            runningTime += frameElapsedTime;
            f64 remainingSeconds = targetFrameRate - frameElapsedTime;
            if (remainingSeconds > 0.0) {
                f64 remainingMs = remainingSeconds * 1000;
                b8 limitFrames = FALSE;
                if (limitFrames) {
                    platformSleep(remainingMs - 1);
                }
                frameCount++;
            }

            input_update(delta_time);
            state->lastTime = currentTime;
        }
    }

    event_unregister(EVENT_CODE_APPLICATION_QUIT, NULL, application_on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, NULL, application_on_event);
    event_unregister(EVENT_CODE_RESIZE, NULL, application_on_resize);

    geometry_system_shutdown(state->geometry_system.block);
    material_system_shutdown();
    texture_system_shutdown();
    renderer_system_shutdown();
    resource_system_shutdown();
    platform_system_shutdown(&state->platform);
    input_system_shutdown(state->input_system.block);
    logger_system_shutdown(state->logger_system.block);
    event_system_shutdown(state->event_system.block);
    memory_system_shutdown();

    return TRUE;
}

b8 application_on_event(u16 code, void const* sender, void const* listener, event_context context)
{
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT:
            LOG_INFO("EVENT_CODE_APPLICATION_QUIT recieved. Shutting down");
            state->running = FALSE;
            return TRUE;
    }
    return FALSE;
}

b8 application_on_key(u16 code, void const* sender, void const* listener, event_context context)
{
    switch (code) {
        case EVENT_CODE_KEY_PRESSED: {
            u16 keyCode = context.as.u16[0];
            switch (keyCode) {
                case KEY_ESCAPE: {
                    LOG_INFO("ESC key pressed");
                    event_context context = {};
                    event_notify(EVENT_CODE_APPLICATION_QUIT, NULL, context);
                    return TRUE;
                }
                default:
                    LOG_INFO("'%c' key pressed", keyCode);
                    return TRUE;
            }
        }
        case EVENT_CODE_KEY_RELEASED: {
            u16 keyCode = context.as.u16[0];
            LOG_INFO("'%c' key released", keyCode);
            return TRUE;
        }
    }
    return FALSE;
}

b8 application_on_resize(u16 code, void const* sender, void const* listener, event_context context)
{
    if (code == EVENT_CODE_RESIZE) {
        i16 width = context.as.i16[0];
        i16 height = context.as.i16[1];

        if (width != state->width || height != state->height) {
            state->width = width;
            state->height = height;

            LOG_DEBUG("application_on_resized: width %d, height %d", width, height);

            if (width == 0 || height == 0) {
                LOG_INFO("Window minimized. Suspending application");
                state->suspended = TRUE;
                return TRUE;
            } else {
                if (state->suspended) {
                    LOG_INFO("Window restored. Resuming application");
                    state->suspended = FALSE;
                }
                state->instance->on_resize(state->instance, width, height);
                renderer_frontend_resize(width, height);
            }
        }
    }
    // Purposely not handled
    return FALSE;
}

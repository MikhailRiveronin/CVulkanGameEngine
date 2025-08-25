#include "application.h"

#include "clock.h"
#include "events.h"
#include "input.h"
#include "game_types.h"
#include "logger.h"
#include "memory/linear_allocator.h"
#include "platform/platform.h"
#include "renderer/renderer_frontend.h"
#include "string_utils.h"
#include "math/kmath.h"

// Systems
#include "systems/texture_system.h"
#include "systems/material_system.h"
#include "systems/geometry_system.h"

typedef struct application_state {
    game* game;
    b8 running;
    b8 suspended;
    i32 width;
    i32 height;
    clock clock;
    f64 lastTime;
    platform_state platform;
    linear_allocator systems_allocator;

    struct {
        u64 required_memory;
        void* memory;
    } memory_system;

    struct {
        u64 required_memory;
        void* memory;
    } logger_system;

    struct {
        u64 required_memory;
        void* memory;
    } event_system;

    struct {
        u64 required_memory;
        void* memory;
    } input_system;

    struct {
        u64 required_memory;
        void* memory;
    } platform_system;

    struct {
        u64 required_memory;
        void* memory;
    } renderer_system;

    struct {
        u64 required_memory;
        void* memory;
    } texture_system;

    struct {
        u64 required_memory;
        void* memory;
    } material_system;

    struct {
        u64 required_memory;
        void* memory;
    } geometry_system;

    // TODO: temp
    geometry* test_geometry;
    // TODO: end temp
} application_state;

static application_state* app_state;

b8 applicationOnEvent(u16 code, void const* sender, void const* listener, event_context context);
b8 applicationOnKey(u16 code, void const* sender, void const* listener, event_context context);
b8 application_on_resize(u16 code, void const* sender, void const* listener, event_context context);

// TODO: temp
b8 event_on_debug_event(u16 code, void* sender, void* listener_inst, event_context data) {
    const char* names[3] = {
        "cobblestone",
        "paving",
        "paving2"};
    static i8 choice = 2;

    // Save off the old name.
    const char* old_name = names[choice];

    choice++;
    choice %= 3;

    // Acquire the new texture.
    if (app_state->test_geometry) {
        app_state->test_geometry->material->diffuse_map.texture = texture_system_acquire_texture(names[choice], TRUE);
        if (!app_state->test_geometry->material->diffuse_map.texture) {
            LOG_WARNING("event_on_debug_event no texture! using default");
            app_state->test_geometry->material->diffuse_map.texture = texture_system_get_default_texture();
        }

        // Release the old texture.
        texture_system_release_texture(old_name);
    }

    return TRUE;
}
// TODO: end temp

b8 application_init(game* game)
{
    if (game->app_state) {
        LOG_ERROR("application_init() called more than once");
        return FALSE;
    }

    game->app_state = memory_allocate(sizeof(*app_state), MEMORY_TAG_APPLICATION);
    app_state = game->app_state;
    app_state->game = game;
    app_state->running = FALSE;
    app_state->suspended = FALSE;

    u64 system_allocator_total_size = 64 * 1024 * 1024;
    linear_allocator_create(system_allocator_total_size, 0, &app_state->systems_allocator);

    // Systems
    memory_system_startup(&app_state->memory_system.required_memory, 0);
    app_state->memory_system.memory = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->memory_system.required_memory);
    if (!memory_system_startup(&app_state->memory_system.required_memory, app_state->memory_system.memory)) {
        LOG_FATAL("application_init: Failed to initialize memory system. Shutting down...");
        return FALSE;
    }

    logger_system_startup(&app_state->logger_system.required_memory, 0);
    app_state->logger_system.memory = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->logger_system.required_memory);
    if (!logger_system_startup(&app_state->logger_system.required_memory, app_state->logger_system.memory)) {
        LOG_FATAL("application_init: Failed to initialize logger system. Shutting down...");
        return FALSE;
    }

    input_system_startup(&app_state->input_system.required_memory, 0);
    app_state->input_system.memory = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->input_system.required_memory);
    if (!input_system_startup(&app_state->input_system.required_memory, app_state->input_system.memory)) {
        LOG_FATAL("application_init: Failed to initialize input system. Shutting down...");
        return FALSE;
    }

    event_system_startup(&app_state->event_system.required_memory, 0);
    app_state->event_system.memory = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->event_system.required_memory);
    if (!event_system_startup(&app_state->event_system.required_memory, app_state->event_system.memory)) {
        LOG_FATAL("application_init: Failed to initialize event system. Shutting down...");
        return FALSE;
    }

    platform_system_startup(&app_state->platform_system.required_memory, 0, 0, 0, 0, 0, 0, 0);
    app_state->platform_system.memory = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->platform_system.required_memory);
    if (!platform_system_startup(
        &app_state->platform_system.required_memory,
        app_state->platform_system.memory,
        &app_state->platform,
        app_state->game->app_config.name,
        app_state->game->app_config.x, app_state->game->app_config.y,
        app_state->game->app_config.width, app_state->game->app_config.height)) {
        LOG_FATAL("application_init: Failed to initialize platform system. Shutting down...");
        return FALSE;
    }

    renderer_system_startup(&app_state->renderer_system.required_memory, 0, 0);
    app_state->renderer_system.memory = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->renderer_system.required_memory);
    if (!renderer_system_startup(
        &app_state->renderer_system.required_memory,
        app_state->renderer_system.memory,
        game->app_config.name)) {
        LOG_FATAL("application_init: Failed to initialize renderer system. Shutting down...");
        return FALSE;
    }

    texture_system_config texture_sys_config;
    texture_sys_config.max_texture_count = 65536;
    texture_system_startup(&app_state->texture_system.required_memory, 0, texture_sys_config);
    app_state->texture_system.memory = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->texture_system.required_memory);
    if (!texture_system_startup(
        &app_state->texture_system.required_memory,
        app_state->texture_system.memory,
        texture_sys_config)) {
        LOG_FATAL("application_init: Failed to initialize texture system. Shutting down...");
        return FALSE;
    }

    material_system_config material_sys_config;
    material_sys_config.max_material_count = 4096;
    material_system_startup(&app_state->material_system.required_memory, 0, material_sys_config);
    app_state->material_system.memory = linear_allocator_allocate(&app_state->systems_allocator, app_state->material_system.required_memory);
    if (!material_system_startup(&app_state->material_system.required_memory, app_state->material_system.memory, material_sys_config)) {
        LOG_FATAL("Failed to initialize material system. Application cannot continue.");
        return FALSE;
    }

    geometry_system_config geometry_sys_config;
    geometry_sys_config.max_geometry_count = 4096;
    geometry_system_startup(&app_state->geometry_system.required_memory, 0, geometry_sys_config);
    app_state->geometry_system.memory = linear_allocator_allocate(&app_state->systems_allocator, app_state->material_system.required_memory);
    if (!geometry_system_startup(&app_state->geometry_system.required_memory, app_state->geometry_system.memory, geometry_sys_config)) {
        LOG_FATAL("Failed to initialize geometry system. Application cannot continue.");
        return FALSE;
    }

    // Load up a plane configuration, and load geometry from it.
    geometry_config g_config = geometry_system_generate_plane_config(15.0f, 5.0f, 5, 5, 5.0f, 2.0f, "test geometry", "test_material");
    app_state->test_geometry = geometry_system_acquire_from_config(g_config, TRUE);

    // Clean up the allocations for the geometry config.
    memory_free(g_config.vertices, sizeof(vertex_3d) * g_config.vertex_count, MEMORY_TAG_ARRAY);
    memory_free(g_config.indices, sizeof(u32) * g_config.index_count, MEMORY_TAG_ARRAY);

    if (!app_state->game->init(app_state->game)) {
        LOG_FATAL("Failed to initialize game");
        return FALSE;
    }

    app_state->width = game->app_config.width;
    app_state->height = game->app_config.height;

    event_register(EVENT_CODE_APPLICATION_QUIT, NULL, applicationOnEvent);
    event_register(EVENT_CODE_KEY_PRESSED, NULL, applicationOnKey);
    event_register(EVENT_CODE_RESIZE, NULL, application_on_resize);
    // TODO: temp
    event_register(EVENT_CODE_DEBUG0, 0, event_on_debug_event);
    // TODO: end temp

    app_state->suspended = FALSE;

    // TODO: What for?
    app_state->game->resize(app_state->game, app_state->width, app_state->height);

    return TRUE;
}

b8 application_run(void)
{
    app_state->running = TRUE;
    memoryPrintUsageStr();

    clock_start(&app_state->clock);
    clock_update(&app_state->clock);
    app_state->lastTime = app_state->clock.elapsed;
    f64 runningTime = 0.0;
    u8 frameCount = 0;
    const f64 targetFrameRate = 1.0 / 60.0;

    while (app_state->running) {
        if (!platformProcMessages(&app_state->platform)) {
            app_state->running = FALSE;
            break;
        }

        if (!app_state->running) {
            break;
        }

        if (!app_state->suspended) {
            clock_update(&app_state->clock);
            f64 currentTime = app_state->clock.elapsed;
            f64 delta_time = currentTime - app_state->lastTime;
            f64 frameStartTime = platform_get_absolute_time();


            if (!app_state->game->update(app_state->game, delta_time)) {
                LOG_FATAL("Game update failed");
                app_state->running = FALSE;
                break;
            }

            if (!app_state->game->render(app_state->game, delta_time)) {
                LOG_FATAL("Game render failed");
                app_state->running = FALSE;
                break;
            }

            // TODO: temporary solution
            render_packet packet;
            packet.delta_time = delta_time;

            // TODO: temp
            geometry_render_data test_render;
            test_render.geometry = app_state->test_geometry;
            test_render.world = mat4_identity();

            packet.geometry_count = 1;
            packet.geometries = &test_render;
            // TODO: end temp

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
            app_state->lastTime = currentTime;
        }
    }

    event_unregister(EVENT_CODE_APPLICATION_QUIT, NULL, applicationOnEvent);
    event_unregister(EVENT_CODE_KEY_PRESSED, NULL, applicationOnEvent);
    event_unregister(EVENT_CODE_RESIZE, NULL, application_on_resize);
    // TODO: temp
    event_unregister(EVENT_CODE_DEBUG0, 0, event_on_debug_event);
    // TODO: end temp

    event_system_shutdown(app_state->event_system.memory);
    input_system_shutdown(app_state->input_system.memory);
    geometry_system_shutdown(app_state->geometry_system.memory);
    material_system_shutdown();
    texture_system_shutdown();
    renderer_system_shutdown();

    platform_system_shutdown(&app_state->platform);

    memory_system_shutdown();

    return TRUE;
}

void applicationGetFramebufferSize(u32* width, u32* height)
{
    *width = app_state->width;
    *height = app_state->height;
}

b8 applicationOnEvent(u16 code, void const* sender, void const* listener, event_context context)
{
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT:
            LOG_INFO("EVENT_CODE_APPLICATION_QUIT recieved. Shutting down");
            app_state->running = FALSE;
            return TRUE;
    }
    return FALSE;
}

b8 applicationOnKey(u16 code, void const* sender, void const* listener, event_context context)
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

        if (width != app_state->width || height != app_state->height) {
            app_state->width = width;
            app_state->height = height;

            LOG_DEBUG("application_on_resized: width %d, height %d", width, height);

            if (width == 0 || height == 0) {
                LOG_INFO("Window minimized. Suspending application");
                app_state->suspended = TRUE;
                return TRUE;
            } else {
                if (app_state->suspended) {
                    LOG_INFO("Window restored. Resuming application");
                    app_state->suspended = FALSE;
                }
                app_state->game->resize(app_state->game, width, height);
                renderer_frontend_resize(width, height);
            }
        }
    }
    // Purposely not handled
    return FALSE;
}

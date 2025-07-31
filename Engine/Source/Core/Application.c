#include "Application.h"
#include "GameTypes.h"
#include "Logger.h"
#include "Events.h"
#include "Input.h"
#include "Clock.h"
#include "Platform/Platform.h"
#include "Renderer/RendererFrontend.h"

typedef struct ApplicationState {
    Game* game;
    b8 running;
    b8 suspended;
    i32 width;
    i32 height;
    Clock clock;
    f64 lastTime;
    PlatformState platform;
} ApplicationState;

static ApplicationState state;
static b8 initialized = FALSE;

b8 applicationOnEvent(u16 code, void const* sender, void const* listener, EventContext context);
b8 applicationOnKey(u16 code, void const* sender, void const* listener, EventContext context);
b8 application_on_resize(u16 code, void const* sender, void const* listener, EventContext context);

b8 applicationInit(Game* game)
{
    if (initialized) {
        LOG_ERROR("applicationInit() called more than once");
        return FALSE;
    }

    state.game = game;

    loggerInit();
    inputInit();

    state.running = TRUE;
    state.suspended = FALSE;

    state.width = game->appConfig.width;
    state.height = game->appConfig.height;

    if (!eventInit()) {
        LOG_ERROR("Failed to init event system");
        return FALSE;
    }

    event_register(EVENT_CODE_APPLICATION_QUIT, NULL, applicationOnEvent);
    event_register(EVENT_CODE_KEY_PRESSED, NULL, applicationOnKey);
    event_register(EVENT_CODE_RESIZE, NULL, application_on_resize);

    if (!platformInit(
        &state.platform, state.game->appConfig.name,
        state.game->appConfig.x, state.game->appConfig.y,
        state.game->appConfig.width, state.game->appConfig.height)) {
        return FALSE;
    }

    if (!rendererInit(game->appConfig.name, &state.platform)) {
        LOG_FATAL("Failed to initialize renderer. Shutting down");
        return FALSE;
    }

    if (!state.game->onInit(state.game)) {
        LOG_FATAL("Failed to initialize game");
        return FALSE;
    }

    // TODO: What for?
    state.game->onResize(state.game, state.width, state.height);

    initialized = TRUE;
    return TRUE;

}

b8 applicationRun(void)
{
    memoryPrintUsageStr();

    clockStart(&state.clock);
    clockUpdate(&state.clock);
    state.lastTime = state.clock.elapsed;
    f64 runningTime = 0.0;
    u8 frameCount = 0;
    const f64 targetFrameRate = 1.0 / 60.0;

    while (state.running) {
        if (!platformProcMessages(&state.platform)) {
            state.running = FALSE;
            break;
        }

        if (!state.running) {
            break;
        }

        if (!state.suspended) {
            clockUpdate(&state.clock);
            f64 currentTime = state.clock.elapsed;
            f64 deltaTime = currentTime - state.lastTime;
            f64 frameStartTime = platform_get_absolute_time();


            if (!state.game->onUpdate(state.game, deltaTime)) {
                LOG_FATAL("Game update failed");
                state.running = FALSE;
                break;
            }

            if (!state.game->onRender(state.game, deltaTime)) {
                LOG_FATAL("Game render failed");
                state.running = FALSE;
                break;
            }

            // TODO: temporary solution
            RenderPacket packet;
            packet.deltaTime = deltaTime;
            renderer_draw_frame(&packet);

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

            inputUpdate(deltaTime);
            state.lastTime = currentTime;
        }
    }

    event_unregister(EVENT_CODE_APPLICATION_QUIT, NULL, applicationOnEvent);
    event_unregister(EVENT_CODE_KEY_PRESSED, NULL, applicationOnEvent);
    event_unregister(EVENT_CODE_RESIZE, NULL, application_on_resize);

    eventDestroy();
    inputDestroy();

    rendererDestroy();

    platformDestroy(&state.platform);

    return TRUE;
}

void applicationGetFramebufferSize(u32* width, u32* height)
{
    *width = state.width;
    *height = state.height;
}

b8 applicationOnEvent(u16 code, void const* sender, void const* listener, EventContext context)
{
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT:
            LOG_INFO("EVENT_CODE_APPLICATION_QUIT recieved. Shutting down");
            state.running = FALSE;
            return TRUE;
    }
    return FALSE;
}

b8 applicationOnKey(u16 code, void const* sender, void const* listener, EventContext context)
{
    switch (code) {
        case EVENT_CODE_KEY_PRESSED: {
            u16 keyCode = context.as.u16[0];
            switch (keyCode) {
                case KEY_ESCAPE: {
                    LOG_INFO("ESC key pressed");
                    EventContext context = {};
                    eventNotify(EVENT_CODE_APPLICATION_QUIT, NULL, context);
                    return TRUE;
                }
                default:
                    LOG_INFO("'%c' key pressed", keyCode);
                    return TRUE;
            }
        }
        case EVENT_CODE_KEY_RELEASED: {
            u16 keyCode = context.as.u16[0];
            LOG_INFO("'%c' key released");
            return TRUE;
        }
    }
    return FALSE;
}

b8 application_on_resize(u16 code, void const* sender, void const* listener, EventContext context)
{
    if (code == EVENT_CODE_RESIZE) {
        i16 width = context.as.i16[0];
        i16 height = context.as.i16[1];

        if (width != state.width || height != state.height) {
            state.width = width;
            state.height = height;

            LOG_DEBUG("application_on_resized: width %d, height %d", width, height);

            if (width == 0 || height == 0) {
                LOG_INFO("Window minimized. Suspending application");
                state.suspended = TRUE;
                return TRUE;
            } else {
                if (state.suspended) {
                    LOG_INFO("Window restored. Resuming application");
                    state.suspended = FALSE;
                }
                state.game->onResize(state.game, width, height);
                renderer_frontend_resize(width, height);
            }
        }
    }
    // Purposely not handled
    return FALSE;
}

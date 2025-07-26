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

    if (!eventInit()) {
        LOG_ERROR("Failed to init event system");
        return FALSE;
    }

    eventRegister(EVENT_CODE_APPLICATION_QUIT, NULL, applicationOnEvent);
    eventRegister(EVENT_CODE_KEY_PRESSED, NULL, applicationOnKey);

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

        if (!state.suspended) {
            clockUpdate(&state.clock);
            f64 currentTime = state.clock.elapsed;
            f64 deltaTime = currentTime - state.lastTime;
            f64 frameStartTime = platformGetAbsoluteTime();


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
            rendererDrawFrame(&packet);

            f64 frameEndTime = platformGetAbsoluteTime();
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

    eventUnregister(EVENT_CODE_APPLICATION_QUIT, NULL, applicationOnEvent);
    eventUnregister(EVENT_CODE_KEY_PRESSED, NULL, applicationOnEvent);

    eventDestroy();
    inputDestroy();

    rendererDestroy();

    platformDestroy(&state.platform);

    return TRUE;
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

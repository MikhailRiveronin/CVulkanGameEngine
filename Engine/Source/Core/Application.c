#include "Application.h"
#include "GameTypes.h"
#include "Logger.h"
#include "Events.h"
#include "Input.h"
#include "Platform/Platform.h"

typedef struct ApplicationState {
    Game* game;
    b8 running;
    b8 suspended;
    i32 width;
    i32 height;
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
    while (state.running) {
        if (!platformProcMessages(&state.platform)) {
            state.running = FALSE;
            break;
        }

        if (!state.suspended) {
            if (!state.game->onUpdate(state.game, 0.0)) {
                LOG_FATAL("Game update failed");
                state.running = FALSE;
                break;
            }

            if (!state.game->onRender(state.game, 0.0)) {
                LOG_FATAL("Game render failed");
                state.running = FALSE;
                break;
            }

            inputUpdate(0.0);
        }
    }

    eventUnregister(EVENT_CODE_APPLICATION_QUIT, NULL, applicationOnEvent);
    eventUnregister(EVENT_CODE_KEY_PRESSED, NULL, applicationOnEvent);

    eventDestroy();
    inputDestroy();
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

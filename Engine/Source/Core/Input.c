#include "Input.h"
#include "Events.h"
#include "logger.h"
#include "memory.h"

typedef struct KeyboardState {
    b8 keys[KEY_ENUM_COUNT];
} KeyboardState;

typedef struct MouseState {
    i16 x;
    i16 y;
    b8 buttons[BUTTON_ENUM_COUNT];
} MouseState;

typedef struct InputState {
    KeyboardState keyboard;
    KeyboardState prevKeyboard;
    MouseState mouse;
    MouseState prevMouse;
} InputState;

static b8 initialized;
static InputState state;

void inputInit()
{
    if (initialized) {
        LOG_ERROR("inputInit() called more then once");
        return;
    }

    initialized = TRUE;
}

void inputDestroy()
{
    initialized = FALSE;
}

void inputUpdate(f64 deltaTime)
{
    if (!initialized) {
        return;
    }

    memoryCopy(&state.prevKeyboard, &state.keyboard, sizeof(state.keyboard));
    memoryCopy(&state.prevMouse, &state.mouse, sizeof(state.mouse));
}

b8 input_is_key_down(Key key)
{
    if (!initialized) {
        return FALSE;
    }
    return state.keyboard.keys[key] == TRUE;
}

b8 input_is_key_up(Key key)
{
    if (!initialized) {
        return TRUE;
    }
    return state.keyboard.keys[key] == FALSE;
}

b8 input_was_key_down(Key key)
{
    if (!initialized) {
        return FALSE;
    }
    return state.prevKeyboard.keys[key] == TRUE;
}

b8 input_was_key_up(Key key)
{
    if (!initialized) {
        return TRUE;
    }
    return state.prevKeyboard.keys[key] == FALSE;
}

void inputProcessKey(Key key, b8 pressed)
{
    switch (key) {
        case KEY_RALT:
            LOG_INFO("Right alt pressed");
            break;
        case KEY_LALT:
            LOG_INFO("Left alt pressed");
            break;
        case KEY_RSHIFT:
            LOG_INFO("Right shift pressed");
            break;
        case KEY_LSHIFT:
            LOG_INFO("Left shift pressed");
            break;
        case KEY_RCONTROL:
            LOG_INFO("Right control pressed");
            break;
        case KEY_LCONTROL:
            LOG_INFO("Left control pressed");
            break;
    }

    if (state.keyboard.keys[key] != pressed) {
        state.keyboard.keys[key] = pressed;

        EventContext context;
        context.as.u16[0] = key;
        eventNotify(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, NULL, context);
    }
}

b8 inputIsButtonDown(Button button)
{
    if (!initialized) {
        return FALSE;
    }
    return state.mouse.buttons[button] == TRUE;
}

b8 inputIsButtonUp(Button button) {
    if (!initialized) {
        return TRUE;
    }
    return state.mouse.buttons[button] == FALSE;
}

b8 inputWasButtonyDown(Button button) {
    if (!initialized) {
        return FALSE;
    }
    return state.prevMouse.buttons[button] == TRUE;
}

b8 inputWasButtonyUp(Button button) {
    if (!initialized) {
        return TRUE;
    }
    return state.prevMouse.buttons[button] == FALSE;
}

void inputGetMousePosition(i16* x, i16* y)
{
    if (!initialized) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = state.mouse.x;
    *y = state.mouse.y;
}

void inputGetPreviousMousePosition(i16* x, i16* y)
{
    if (!initialized) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = state.prevMouse.x;
    *y = state.prevMouse.y;
}

void inputProcessButton(Button button, b8 pressed)
{
    if (state.mouse.buttons[button] != pressed) {
        state.mouse.buttons[button] = pressed;

        EventContext context;
        context.as.u16[0] = button;
        eventNotify(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, NULL, context);
    }
}

void inputProcessMouseMove(i16 x, i16 y)
{
    if (state.mouse.x != x || state.mouse.y != y) {
        // LOG_DEBUG("Process mouse move %d %d", x, y);

        state.mouse.x = x;
        state.mouse.y = y;

        EventContext context;
        context.as.u16[0] = x;
        context.as.u16[1] = y;
        eventNotify(EVENT_CODE_MOUSE_MOVED, NULL, context);
    }
}

void inputProcessMouseWheel(i8 zDelta)
{
    EventContext context;
    context.as.i8[0] = zDelta;
    eventNotify(EVENT_CODE_MOUSE_WHEEL, NULL, context);
}

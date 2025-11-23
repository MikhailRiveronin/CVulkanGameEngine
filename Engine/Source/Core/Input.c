#include "input.h"
#include "event_system.h"
#include "logger.h"
#include "memory_system.h"

typedef struct KeyboardState {
    b8 keys[KEY_ENUM_COUNT];
} KeyboardState;

typedef struct MouseState {
    i16 x;
    i16 y;
    b8 buttons[BUTTON_ENUM_COUNT];
} MouseState;

typedef struct input_state {
    KeyboardState keyboard;
    KeyboardState prevKeyboard;
    MouseState mouse;
    MouseState prevMouse;
} input_state;
static input_state* system_state;

b8 input_system_startup(u64* memory_size, void* memory)
{
    *memory_size = sizeof(*system_state);
    if (!memory) {
        return TRUE;
    }

    system_state = memory;
    return TRUE;
}

void input_system_shutdown(void* memory)
{
}

void input_update(f64 deltaTime)
{
    if (!system_state) {
        return;
    }

    memory_system_copy(&system_state->prevKeyboard, &system_state->keyboard, sizeof(system_state->keyboard));
    memory_system_copy(&system_state->prevMouse, &system_state->mouse, sizeof(system_state->mouse));
}

b8 input_is_key_down(Key key)
{
    if (!system_state)
    {
        return FALSE;
    }

    return system_state->keyboard.keys[key] == TRUE;
}

b8 input_is_key_up(Key key)
{
    if (!system_state)
    {
        return TRUE;
    }

    return system_state->keyboard.keys[key] == FALSE;
}

b8 input_was_key_down(Key key)
{
    if (!system_state) {
        return FALSE;
    }
    return system_state->prevKeyboard.keys[key] == TRUE;
}

b8 input_was_key_up(Key key)
{
    if (!system_state) {
        return TRUE;
    }
    return system_state->prevKeyboard.keys[key] == FALSE;
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

    if (system_state->keyboard.keys[key] != pressed) {
        system_state->keyboard.keys[key] = pressed;

        event_context context;
        context.as.u16[0] = key;
        event_notify(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, NULL, context);
    }
}

b8 input_is_button_down(Button button)
{
    if (!system_state)
    {
        return FALSE;
    }

    return system_state->mouse.buttons[button] == TRUE;
}

b8 input_is_button_up(Button button)
{
    if (!system_state)
    {
        return TRUE;
    }

    return system_state->mouse.buttons[button] == FALSE;
}

b8 inputWasButtonyDown(Button button) {
    if (!system_state) {
        return FALSE;
    }
    return system_state->prevMouse.buttons[button] == TRUE;
}

b8 inputWasButtonyUp(Button button) {
    if (!system_state) {
        return TRUE;
    }
    return system_state->prevMouse.buttons[button] == FALSE;
}

void inputGetMousePosition(i16* x, i16* y)
{
    if (!system_state) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = system_state->mouse.x;
    *y = system_state->mouse.y;
}

void inputGetPreviousMousePosition(i16* x, i16* y)
{
    if (!system_state) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = system_state->prevMouse.x;
    *y = system_state->prevMouse.y;
}

void inputProcessButton(Button button, b8 pressed)
{
    if (system_state->mouse.buttons[button] != pressed) {
        system_state->mouse.buttons[button] = pressed;

        event_context context;
        context.as.u16[0] = button;
        event_notify(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, NULL, context);
    }
}

void inputProcessMouseMove(i16 x, i16 y)
{
    if (system_state->mouse.x != x || system_state->mouse.y != y) {
        // LOG_DEBUG("Process mouse move %d %d", x, y);

        system_state->mouse.x = x;
        system_state->mouse.y = y;

        event_context context;
        context.as.u16[0] = x;
        context.as.u16[1] = y;
        event_notify(EVENT_CODE_MOUSE_MOVED, NULL, context);
    }
}

void inputProcessMouseWheel(i8 zDelta)
{
    event_context context;
    context.as.i8[0] = zDelta;
    event_notify(EVENT_CODE_MOUSE_WHEEL, NULL, context);
}

#pragma once

#include "defines.h"

typedef struct event_context {
    union {
        i64 i64[2];
        u64 u64[2];
        f64 f64[2];

        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];

        char c[16];
    } as;
} event_context;


// System internal event codes. Application should use codes beyond 255.
typedef enum SystemEventCode {
    // Shuts the application down on the next frame.
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    // Keyboard key pressed.
    /* Context usage:
     * u16 key_code = data.data.u16[0];
     */
    EVENT_CODE_KEY_PRESSED = 0x02,

    // Keyboard key released.
    /* Context usage:
     * u16 key_code = data.data.u16[0];
     */
    EVENT_CODE_KEY_RELEASED = 0x03,

    // Mouse button pressed.
    /* Context usage:
     * u16 button = data.data.u16[0];
     */
    EVENT_CODE_BUTTON_PRESSED = 0x04,

    // Mouse button released.
    /* Context usage:
     * u16 button = data.data.u16[0];
     */
    EVENT_CODE_BUTTON_RELEASED = 0x05,

    // Mouse moved.
    /* Context usage:
     * u16 x = data.data.u16[0];
     * u16 y = data.data.u16[1];
     */
    EVENT_CODE_MOUSE_MOVED = 0x06,

    // Mouse wheel.
    /* Context usage:
     * u8 z_delta = data.data.u8[0];
     */
    EVENT_CODE_MOUSE_WHEEL = 0x07,

    // Resized/resolution changed from the OS.
    /* Context usage:
     * u16 width = data.data.u16[0];
     * u16 height = data.data.u16[1];
     */
    EVENT_CODE_RESIZE = 0x08,

    EVENT_CODE_DEBUG0 = 0x10,
    EVENT_CODE_DEBUG1 = 0x11,
    EVENT_CODE_DEBUG2 = 0x12,
    EVENT_CODE_DEBUG3 = 0x13,
    EVENT_CODE_DEBUG4 = 0x14,

    EVENT_CODE_ENUM_MAX = 0xFF
} SystemEventCode;

typedef b8 (* pfn_on_event)(u16 code, void const* sender, void const* listener, event_context context);

b8 event_system_startup(u64* memory_size, void* memory);
void event_system_shutdown(void* memory);

/**
 * Register to listen for when events are sent with the provided code. Events with duplicate
 * listener/on_event combos will not be registered again and will cause this to return FALSE.
 * @param code The event code to listen for.
 * @param listener A pointer to a listener instance. Can be 0/NULL.
 * @param on_event The on_event function pointer to be invoked when the event code is fired.
 * @return TRUE if the event is successfully registered; otherwise false.
 */
LIB_API b8 event_register(u16 code, void* listener, pfn_on_event on_event);

/**
 * Unregister from listening for when events are sent with the provided code. If no matching
 * registration is found, this function returns FALSE.
 * @param code The event code to stop listening for.
 * @param listener A pointer to a listener instance. Can be 0/NULL.
 * @param on_event The on_event function pointer to be unregistered.
 * @return TRUE if the event is successfully unregistered; otherwise false.
 */
LIB_API b8 event_unregister(u16 code, void const* listener, pfn_on_event on_event);

/**
 * Fires an event to listeners of the given code. If an event handler returns 
 * TRUE, the event is considered handled and is not passed on to any more listeners.
 * @param code The event code to fire.
 * @param sender A pointer to the sender. Can be 0/NULL.
 * @param context The event data.
 * @return TRUE if handled, otherwise FALSE.
 */
LIB_API b8 event_notify(u16 code, void const* sender, event_context context);

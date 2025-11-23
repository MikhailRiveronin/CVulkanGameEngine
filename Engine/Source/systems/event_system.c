#include "event_system.h"
#include "core/logger.h"
#include "containers/darray.h"

typedef struct registered_listener {
    void* listener;
    pfn_on_event callback;
} registered_listener;

typedef struct event_code_entry {
    DARRAY(registered_listener) listeners;
} event_code_entry;

#define MAX_EVENT_CODES 1024

typedef struct event_system_state {
    event_code_entry event_codes[MAX_EVENT_CODES];
} event_system_state;
static event_system_state* system_state;

b8 event_system_startup(u64* memory_size, void* memory)
{
    *memory_size = sizeof(*system_state);
    if (!memory) {
        return TRUE;
    }

    system_state = memory;
    return TRUE;
}

void event_system_shutdown(void* memory)
{
    for (u32 i = 0; i < MAX_EVENT_CODES; ++i) {
        if (system_state->event_codes[i].listeners.size != 0) {
            DARRAY_DESTROY(system_state->event_codes[i].listeners);
        }
    }
}

b8 event_register(u16 code, void* listener, pfn_on_event on_event)
{
    if (!system_state) {
        LOG_ERROR("registerEvent() called before then event system is initialized");
        return FALSE;
    }

    if (system_state->event_codes[code].listeners.capacity == 0) {
        DARRAY_INIT(system_state->event_codes[code].listeners, MEMORY_TAG_APPLICATION);
    }

    for (u32 i = 0; i < system_state->event_codes[code].listeners.size; ++i) {
        registered_listener registered = DARRAY_AT(system_state->event_codes[code].listeners, i);
        if (registered.listener == listener && registered.callback == on_event) {
            LOG_ERROR("Event has already been registered");
            return FALSE;
        }
    }

    registered_listener registered;
    registered.listener = listener;
    registered.callback = on_event;
    DARRAY_PUSH(system_state->event_codes[code].listeners, registered);
    return TRUE;
}

b8 event_unregister(u16 code, void const* listener, pfn_on_event on_event)
{
    if (!system_state) {
        LOG_ERROR("unregisterEvent() called before then event system is initialized");
        return FALSE;
    }

    if (system_state->event_codes[code].listeners.capacity == 0) {
        LOG_ERROR("No listeners registered for event code %u", code);
        return FALSE;
    }

    for (u32 i = 0; i < system_state->event_codes[code].listeners.size; ++i) {
        registered_listener unregistered = DARRAY_AT(system_state->event_codes[code].listeners, i);
        if (unregistered.listener == listener && unregistered.callback == on_event) {
            registered_listener unregistered;
            DARRAY_ERASE(system_state->event_codes[code].listeners, i, unregistered);
            return TRUE;
        }
    }
    return FALSE;
}

b8 event_notify(u16 code, void const* sender, event_context context)
{
    if (!system_state) {
        LOG_ERROR("eventNotify() called before then event system is initialized");
        return FALSE;
    }

    if (system_state->event_codes[code].listeners.capacity == 0) {
        // LOG_ERROR("No listeners registered for event code %u", code);
        return FALSE;
    }

    for (u32 i = 0; i < system_state->event_codes[code].listeners.size; ++i) {
        registered_listener registered = DARRAY_AT(system_state->event_codes[code].listeners, i);
        if (registered.callback(code, sender, registered.listener, context)) {
            return TRUE;
        }
    }
    return FALSE;
}

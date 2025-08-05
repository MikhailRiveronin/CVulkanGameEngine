#include "Events.h"
#include "core/logger.h"
#include "Containers/darray.h"

typedef struct RegisteredListener {
    void* listener;
    OnEventCallback callback;
} RegisteredListener;

typedef struct EventCodeEntry {
    DARRAY(RegisteredListener) listeners;
} EventCodeEntry;

#define MAX_EVENT_CODES 1024

typedef struct EventSystemState {
    EventCodeEntry eventCodes[MAX_EVENT_CODES];
} EventSystemState;

static b8 initialized = FALSE;
static EventSystemState state;

b8 eventInit()
{
    if (initialized) {
        LOG_ERROR("eventSystemInit() called more then once");
        return FALSE;
    }
    initialized = TRUE;
    return TRUE;
}

void eventDestroy()
{
    for (u32 i = 0; i < MAX_EVENT_CODES; ++i) {
        if (state.eventCodes[i].listeners.size != 0) {
            DARRAY_DESTROY(state.eventCodes[i].listeners);
        }
    }
}

b8 event_register(u16 code, void* listener, OnEventCallback onEvent)
{
    if (!initialized) {
        LOG_ERROR("registerEvent() called before then event system is initialized");
        return FALSE;
    }

    if (state.eventCodes[code].listeners.capacity == 0) {
        DARRAY_INIT(state.eventCodes[code].listeners, MEMORY_TAG_APPLICATION);
    }

    for (u32 i = 0; i < state.eventCodes[code].listeners.size; ++i) {
        RegisteredListener registered = DARRAY_AT(state.eventCodes[code].listeners, i);
        if (registered.listener == listener && registered.callback == onEvent) {
            LOG_ERROR("Event has already been registered");
            return FALSE;
        }
    }

    RegisteredListener registered;
    registered.listener = listener;
    registered.callback = onEvent;
    DARRAY_PUSH(state.eventCodes[code].listeners, registered);
    return TRUE;
}

b8 event_unregister(u16 code, void const* listener, OnEventCallback onEvent)
{
    if (!initialized) {
        LOG_ERROR("unregisterEvent() called before then event system is initialized");
        return FALSE;
    }

    if (state.eventCodes[code].listeners.capacity == 0) {
        LOG_ERROR("No listeners registered for event code %u", code);
        return FALSE;
    }

    for (u32 i = 0; i < state.eventCodes[code].listeners.size; ++i) {
        RegisteredListener unregistered = DARRAY_AT(state.eventCodes[code].listeners, i);
        if (unregistered.listener == listener && unregistered.callback == onEvent) {
            RegisteredListener unregistered;
            DARRAY_ERASE(state.eventCodes[code].listeners, i, unregistered);
            return TRUE;
        }
    }
    return FALSE;
}

b8 eventNotify(u16 code, void const* sender, EventContext context)
{
    if (!initialized) {
        LOG_ERROR("eventNotify() called before then event system is initialized");
        return FALSE;
    }

    if (state.eventCodes[code].listeners.capacity == 0) {
        // LOG_ERROR("No listeners registered for event code %u", code);
        return FALSE;
    }

    for (u32 i = 0; i < state.eventCodes[code].listeners.size; ++i) {
        RegisteredListener registered = DARRAY_AT(state.eventCodes[code].listeners, i);
        if (registered.callback(code, sender, registered.listener, context)) {
            return TRUE;
        }
    }
    return FALSE;
}

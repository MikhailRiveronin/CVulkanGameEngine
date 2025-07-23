#include "Events.h"
#include "Core/Logger.h"
#include "Core/Memory.h"
#include "Containers/DynamicArray.h"

typedef struct RegisteredListener {
    void* listener;
    OnEventCallback callback;
} RegisteredListener;

typedef struct EventCodeEntry {
    RegisteredListener* listeners;
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

    memoryZero(&state, sizeof(state));
    initialized = TRUE;
    return TRUE;
}

void eventDestroy()
{
    for (u32 i = 0; i < MAX_EVENT_CODES; ++i) {
        if (state.eventCodes[i].listeners) {
            DYNAMIC_ARRAY_DESTROY(state.eventCodes[i].listeners);
            state.eventCodes[i].listeners = NULL;
        }
    }
}

b8 eventRegister(u16 code, void* listener, OnEventCallback onEvent)
{
    if (!initialized) {
        LOG_ERROR("registerEvent() called before then event system is initialized");
        return FALSE;
    }

    if (!state.eventCodes[code].listeners) {
        state.eventCodes[code].listeners = DYNAMIC_ARRAY_CREATE(RegisteredListener);
    }

    for (u32 i = 0; i < DYNAMIC_ARRAY_LENGTH(state.eventCodes[code].listeners); ++i) {
        RegisteredListener registered = state.eventCodes[code].listeners[i];
        if (registered.listener == listener && registered.callback == onEvent) {
            LOG_ERROR("Event has already been registered");
            return FALSE;
        }
    }

    RegisteredListener registered;
    registered.listener = listener;
    registered.callback = onEvent;
    DYNAMIC_ARRAY_PUSH(state.eventCodes[code].listeners, registered);
    return TRUE;
}

b8 eventUnregister(u16 code, void const* listener, OnEventCallback onEvent)
{
    if (!initialized) {
        LOG_ERROR("unregisterEvent() called before then event system is initialized");
        return FALSE;
    }

    if (!state.eventCodes[code].listeners) {
        LOG_ERROR("No listeners registered for event code %u", code);
        return FALSE;
    }

    for (u32 i = 0; i < DYNAMIC_ARRAY_LENGTH(state.eventCodes[code].listeners); ++i) {
        RegisteredListener unregistered = state.eventCodes[code].listeners[i];
        if (unregistered.listener == listener && unregistered.callback == onEvent) {
            RegisteredListener unregistered;
            DYNAMIC_ARRAY_ERASE(state.eventCodes[code].listeners, i, &unregistered);
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

    if (!state.eventCodes[code].listeners) {
        // LOG_ERROR("No listeners registered for event code %u", code);
        return FALSE;
    }

    for (u32 i = 0; i < DYNAMIC_ARRAY_LENGTH(state.eventCodes[code].listeners); ++i) {
        RegisteredListener registered = state.eventCodes[code].listeners[i];
        if (registered.callback(code, sender, registered.listener, context)) {
            return TRUE;
        }
    }
    return FALSE;
}

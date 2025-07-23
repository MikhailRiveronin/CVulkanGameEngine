#include "Platform/Platform.h"
#include "Core/Logger.h"
#include "Core/Input.h"
#include "Core/Events.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

typedef struct Win32State {
    HINSTANCE hInstance;
    HWND hWnd;
} Win32State;

static u16 charAttributes[] = {
    FOREGROUND_RED,
    FOREGROUND_RED,
    FOREGROUND_RED | FOREGROUND_GREEN,
    FOREGROUND_GREEN,
    FOREGROUND_GREEN,
    FOREGROUND_GREEN };

static f64 countRate;
static LARGE_INTEGER startTime;

LRESULT CALLBACK windowProc(HWND hWnd, u32 message, WPARAM wParam, LPARAM lParam);

b8 platformInit(PlatformState* platformState, char const* appName, i32 x, i32 y, i32 width, i32 height)
{
    platformState->specific = malloc(sizeof(Win32State));
    Win32State* state = (Win32State*)platformState->specific;
    state->hInstance = GetModuleHandleA(NULL);

    WNDCLASSEXA wc;
    platformZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = windowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state->hInstance;
    wc.hIcon = LoadIconA(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "WindowClass";
    wc.hIconSm = NULL;
    if (!RegisterClassExA(&wc)) {
        MessageBoxExA(NULL, "Failed to register window class", "Error", MB_ICONERROR | MB_OK, 0);
        LOG_FATAL("Failed to register window class");
        return FALSE;
    }

    RECT rect;
    platformZeroMemory(&rect, sizeof(rect));
    rect.left = x;
    rect.top = y;
    rect.right = width;
    rect.bottom = height;

    u32 style = WS_OVERLAPPEDWINDOW;
    u32 exStyle = WS_EX_APPWINDOW;
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    state->hWnd = CreateWindowExA(
        exStyle, "WindowClass", appName, style,
        rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, state->hInstance, NULL);
    if (!state->hWnd)
    {
        MessageBoxExA(NULL, "Failed to create window", "Error", MB_ICONERROR | MB_OK, 0);
        LOG_FATAL("Failed to create window");
        return FALSE;
    }

    ShowWindow(state->hWnd, SW_SHOWNORMAL);

    LARGE_INTEGER countsPerSecond;
    QueryPerformanceFrequency(&countsPerSecond);
    countRate = 1.0 / (f64)countsPerSecond.QuadPart;
    QueryPerformanceCounter(&startTime);

    return TRUE;
}

void platformDestroy(PlatformState* platformState)
{
    Win32State* state = (Win32State*)platformState->specific;
    if (state->hWnd) {
        DestroyWindow(state->hWnd);
        state->hWnd = NULL;
    }
}

b8 platformProcMessages(PlatformState* platformState)
{
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            EventContext context = {};
            eventNotify(EVENT_CODE_APPLICATION_QUIT, NULL, context);
            break;
        }

        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return TRUE;
}

void* platformAllocate(u64 size, b8 aligned)
{
    return malloc(size);
}

void platformFree(void* ptr, b8 aligned)
{
    free(ptr);
}

void* platformSetMemory(void* dest, i32 value, u64 size)
{
    return memset(dest, value, size);
}

void* platformZeroMemory(void* dest, u64 size)
{
    return platformSetMemory(dest, 0, size);
}

void* platformCopyMemory(void* dest, void const* src, u64 size)
{
    return memcpy(dest, src, size);
}

void platformWriteConsoleOutput(char const* message, u8 color)
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    u32 numCharsWritten;
    SetConsoleTextAttribute(handle, charAttributes[color]);
    WriteConsoleA(handle, message, strlen(message), &numCharsWritten, NULL);

    OutputDebugStringA(message);
}

void platformWriteConsoleError(char const* message, u8 color)
{
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    u32 numCharsWritten;
    SetConsoleTextAttribute(handle, charAttributes[color]);
    WriteConsoleA(handle, message, strlen(message), &numCharsWritten, NULL);

    OutputDebugStringA(message);
}

void platformWriteError(const char *message)
{
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    u32 numCharsWritten;
    WriteConsoleA(handle, message, strlen(message), &numCharsWritten, NULL);
}

f64 platformGetAbsoluteTime()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (f64)now.QuadPart * countRate;
}

void platformSleep(u64 ms)
{
    Sleep(ms);
}

LRESULT CALLBACK windowProc(HWND hWnd, u32 message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            RECT rect;
            GetClientRect(hWnd, &rect);
            i32 width = rect.right - rect.left;
            i32 height = rect.bottom - rect.top;
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            b8 pressed = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
            u16 key = (u16)wParam;
            inputProcessKey(key, pressed);
            return 0;
        }
        case WM_MOUSEMOVE: {
            i16 x = GET_X_LPARAM(lParam);
            i16 y = GET_Y_LPARAM(lParam);
            inputProcessMouseMove(x, y);
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            b8 pressed = message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN;
            Button button;
            switch (message) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    button = BUTTON_LEFT;
                    inputProcessButton(button, pressed);
                    return 0;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    button = BUTTON_MIDDLE;
                    inputProcessButton(button, pressed);
                    return 0;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    button = BUTTON_RIGHT;
                    inputProcessButton(button, pressed);
                    return 0;
            }
            return 0;
        }
        case WM_MOUSEWHEEL: {
            i16 zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (zDelta != 0) {
                // Flatten the input to an OS-independent (-1, 1).
                zDelta = (zDelta < 0) ? -1 : 1;
                inputProcessMouseWheel(zDelta);
            }
            return 0;
        }
    }

    return DefWindowProcA(hWnd, message, wParam, lParam);
}

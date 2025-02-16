#include "Platform/Platform.h"

#include "Core/Logger.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// #include <stdlib.h>

typedef struct Win32State
{
    HINSTANCE hInstance;
    HWND hWnd;
} Win32State;

static f64 secondsPerCount;
static LARGE_INTEGER startTimeCounts;

LRESULT CALLBACK windowProc(HWND hWnd, u32 message, WPARAM wParam, LPARAM lParam);

b8 platformInit(PlatformState *platformState, const char *appName, i32 x, i32 y, i32 width, i32 height)
{
    platformState->state = malloc(sizeof(Win32State));
    Win32State *state = (Win32State *)platformState->state;
    state->hInstance = GetModuleHandleA(NULL);

    WNDCLASSEXA wc;
    memset(&wc, 0, sizeof(WNDCLASSEXA));
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
    wc.lpszClassName = "Window class";
    wc.hIconSm = NULL;

    if (!RegisterClassExA(&wc))
    {
        MessageBoxExA(NULL, "Failed to register window class", "Error", MB_ICONERROR | MB_OK, 0);
        LOG_FATAL("Failed to register window class");
        return FALSE;
    }

    RECT rect = { x, y, width, height };
    u32 style = WS_OVERLAPPEDWINDOW;
    u32 exStyle = WS_EX_APPWINDOW;
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    HWND hWnd = CreateWindowExA(exStyle, "Window class", appName, style, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, state->hInstance, NULL);
    if (!hWnd)
    {
        MessageBoxExA(NULL, "Failed to create window", "Error", MB_ICONERROR | MB_OK, 0);
        LOG_FATAL("Failed to create window");
        return FALSE;
    }
    else
    {
        state->hWnd = hWnd;
    }

    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    LARGE_INTEGER countsPerSecond;
    QueryPerformanceFrequency(&countsPerSecond);
    secondsPerCount = 1. / (f64)countsPerSecond.QuadPart;
    QueryPerformanceCounter(&startTimeCounts);

    return TRUE;
}

void platformTerminate(PlatformState *platformState)
{
    Win32State *state = (Win32State *)platformState->state;
    if (state->hWnd) {
        DestroyWindow(state->hWnd);
        state->hWnd = NULL;
    }
}

b8 platformProcMessages(PlatformState *platformState)
{
    MSG msg;
    if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return TRUE;
}

void *platformAllocate(u64 size, b8 isAligned)
{
    return malloc(size);
}

void platformFree(void *ptr, b8 isAligned)
{
    free(ptr);
}

void *platformZeroMemory(void *ptr, u64 size)
{
    return memset(ptr, 0, size);
}

void *platformSetMemory(void *ptr, i32 value, u64 size)
{
    return memset(ptr, value, size);
}

void *platformCopyMemory(void *dest, const void *src, u64 size)
{
    return memcpy(dest, src, size);
}

void platformWriteConsole(const char *message, b8 isError)
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    u8 charAttributes = isError ? FOREGROUND_RED : FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    SetConsoleTextAttribute(handle, charAttributes);
    u32 numCharsWritten;
    WriteConsoleA(handle, message, strlen(message), &numCharsWritten, NULL);
}

void platformWriteError(const char *message)
{
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    u32 numCharsWritten;
    WriteConsoleA(handle, message, strlen(message), &numCharsWritten, NULL);
}

f64 platformGetTime();
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (f64)now.QuadPart * secondsPerCount;
}

void platformSleep(u64 ms)
{
    Sleep(ms);
}

LRESULT CALLBACK WindowProc(HWND hWnd, u32 message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_ERASEBKGND:
            return 1;
        case WM_QUIT:
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            RECT rect;
            GetClientRect(hWnd, &rect);
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            b8 isPressed = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
        } break;
        case WM_MOUSEMOVE: {
            // Mouse move
            //i32 x_position = GET_X_LPARAM(l_param);
            //i32 y_position = GET_Y_LPARAM(l_param);
            // TODO: input processing.
        } break;
        case WM_MOUSEWHEEL: {
            // i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            // if (z_delta != 0) {
            //     // Flatten the input to an OS-independent (-1, 1)
            //     z_delta = (z_delta < 0) ? -1 : 1;
            //     // TODO: input processing.
            // }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            //b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            // TODO: input processing.
        } break;
    }

    return DefWindowProcA(hWnd, message, wParam, lParam);
}

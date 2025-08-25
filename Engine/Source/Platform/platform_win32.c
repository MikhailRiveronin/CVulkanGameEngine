#include "platform/platform.h"
#include "core/logger.h"
#include "core/input.h"
#include "core/events.h"
#include "renderer/vulkan/vulkan_types.inl"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

typedef struct platform_system_state {
    HINSTANCE hInstance;
    HWND hWnd;
} platform_system_state;
static platform_system_state* system_state;

static u16 charAttributes[] = {
    FOREGROUND_RED,
    FOREGROUND_RED,
    FOREGROUND_RED | FOREGROUND_GREEN,
    FOREGROUND_GREEN,
    FOREGROUND_GREEN,
    FOREGROUND_GREEN };

static f64 count_rate;
static LARGE_INTEGER start_time;

LRESULT CALLBACK windowProc(HWND hWnd, u32 message, WPARAM wParam, LPARAM lParam);
static void setup_clock();

b8 platform_system_startup(u64* memory_size, void* memory, platform_state* plat_state, char const* appName, i32 x, i32 y, i32 width, i32 height)
{
    *memory_size = sizeof(*system_state);
    if (!memory) {
        return TRUE;
    }

    system_state = memory;
    plat_state->specific = system_state;
    system_state->hInstance = GetModuleHandleA(NULL);

    WNDCLASSEXA wc;
    platform_zero_memory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = windowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = system_state->hInstance;
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
    platform_zero_memory(&rect, sizeof(rect));
    rect.left = x;
    rect.top = y;
    rect.right = width;
    rect.bottom = height;

    u32 style = WS_OVERLAPPEDWINDOW;
    u32 exStyle = WS_EX_APPWINDOW;
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    system_state->hWnd = CreateWindowExA(
        exStyle, "WindowClass", appName, style,
        rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, system_state->hInstance, NULL);
    if (!system_state->hWnd)
    {
        MessageBoxExA(NULL, "Failed to create window", "Error", MB_ICONERROR | MB_OK, 0);
        LOG_FATAL("Failed to create window");
        return FALSE;
    }

    ShowWindow(system_state->hWnd, SW_SHOWNORMAL);

    setup_clock();

    return TRUE;
}

void platform_system_shutdown(platform_state* plat_state)
{
    if (system_state->hWnd) {
        DestroyWindow(system_state->hWnd);
        system_state->hWnd = NULL;
    }
}

b8 platformProcMessages(platform_state* plat_state)
{
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            event_context context = {};
            event_notify(EVENT_CODE_APPLICATION_QUIT, NULL, context);
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

void* platform_zero_memory(void* dest, u64 size)
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
    DWORD numCharsWritten;
    SetConsoleTextAttribute(handle, charAttributes[color]);
    WriteConsoleA(handle, message, strlen(message), &numCharsWritten, NULL);

    OutputDebugStringA(message);
}

void platformWriteConsoleError(char const* message, u8 color)
{
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    DWORD numCharsWritten;
    SetConsoleTextAttribute(handle, charAttributes[color]);
    WriteConsoleA(handle, message, strlen(message), (LPDWORD)&numCharsWritten, NULL);

    OutputDebugStringA(message);
}

void platformWriteError(char const *message)
{
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    DWORD numCharsWritten;
    WriteConsoleA(handle, message, strlen(message), &numCharsWritten, NULL);
}

f64 platform_get_absolute_time()
{
    if (system_state == 0) {
        setup_clock();
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (f64)now.QuadPart * count_rate;
}

void platformSleep(u64 ms)
{
    Sleep(ms);
}

b8 platform_create_vulkan_surface(vulkan_context* context)
{
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.hinstance = system_state->hInstance;
    createInfo.hwnd = system_state->hWnd;
    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &createInfo, context->allocator, &context->surface);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create surface");
        return FALSE;
    }

    LOG_DEBUG("Surface created");
    return TRUE;
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

            event_context context;
            context.as.i16[0] = width;
            context.as.i16[1] = height;
            event_notify(EVENT_CODE_RESIZE, NULL, context);
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

            if (wParam == VK_MENU) {
                if (GetKeyState(VK_RMENU) & 0x8000) {
                    key = KEY_RALT;
                }
                if (GetKeyState(VK_LMENU) & 0x8000) {
                    key = KEY_LALT;
                }
            }

            if (wParam == VK_SHIFT) {
                if (GetKeyState(VK_RSHIFT) & 0x8000) {
                    key = KEY_RSHIFT;
                }
                if (GetKeyState(VK_LSHIFT) & 0x8000) {
                    key = KEY_LSHIFT;
                }
            }

            if (wParam == VK_CONTROL) {
                if (GetKeyState(VK_RCONTROL) & 0x8000) {
                    key = KEY_RCONTROL;
                }
                if (GetKeyState(VK_LCONTROL) & 0x8000) {
                    key = KEY_LCONTROL;
                }
            }

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

void setup_clock()
{
    LARGE_INTEGER countsPerSecond;
    QueryPerformanceFrequency(&countsPerSecond);
    count_rate = 1.0 / (f64)countsPerSecond.QuadPart;
    QueryPerformanceCounter(&start_time);
}

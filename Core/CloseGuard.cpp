#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "CloseGuard.h"

namespace
{
constexpr DWORD WINDOW_POLL_INTERVAL_MS = 500;
constexpr DWORD GUARD_POLL_INTERVAL_MS = 1000;
constexpr int REAPPEAR_ATTEMPTS = 6;
constexpr DWORD GRACE_PERIOD_MS = 3000;
constexpr int UNNEUTER_DELAY_TICKS = 20;

HHOOK g_hook = nullptr;
HANDLE g_close_event = nullptr;
HWND g_window = nullptr;
LONG g_quit_armed = 0;
void* g_terminate_proc = nullptr;
void* g_nt_terminate_proc = nullptr;
BYTE g_terminate_byte = 0;
BYTE g_nt_terminate_byte = 0;
int g_unneuter_state = 0;
int g_unneuter_countdown = 0;

void capture_terminate_prologue()
{
    const HMODULE kernel = GetModuleHandleW(L"kernel32.dll");
    if (kernel != nullptr)
    {
        g_terminate_proc = reinterpret_cast<void*>(GetProcAddress(kernel, "TerminateProcess"));
        if (g_terminate_proc != nullptr)
        {
            g_terminate_byte = *static_cast<BYTE*>(g_terminate_proc);
            if (g_terminate_byte == 0xC3)
            {
                g_terminate_byte = 0x48;
            }
        }
    }
    const HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (ntdll != nullptr)
    {
        g_nt_terminate_proc = reinterpret_cast<void*>(GetProcAddress(ntdll, "NtTerminateProcess"));
        if (g_nt_terminate_proc != nullptr)
        {
            g_nt_terminate_byte = *static_cast<BYTE*>(g_nt_terminate_proc);
            if (g_nt_terminate_byte == 0xC3)
            {
                g_nt_terminate_byte = 0x4c;
            }
        }
    }
}

void restore_prologue_byte(void* proc, BYTE original)
{
    if (proc == nullptr)
    {
        return;
    }
    DWORD protect = 0;
    if (VirtualProtect(proc, 1, PAGE_EXECUTE_READWRITE, &protect))
    {
        *static_cast<BYTE*>(proc) = original;
        DWORD restored = 0;
        VirtualProtect(proc, 1, protect, &restored);
    }
}

void restore_self_terminate()
{
    restore_prologue_byte(g_terminate_proc, g_terminate_byte);
    restore_prologue_byte(g_nt_terminate_proc, g_nt_terminate_byte);
}

void maybe_unneuter()
{
    if (g_unneuter_state >= 2 || g_nt_terminate_proc == nullptr)
    {
        return;
    }
    if (g_unneuter_state == 0)
    {
        if (*static_cast<unsigned char*>(g_nt_terminate_proc) == 0xC3)
        {
            g_unneuter_state = 1;
            g_unneuter_countdown = UNNEUTER_DELAY_TICKS;
        }
        return;
    }
    if (--g_unneuter_countdown <= 0)
    {
        restore_self_terminate();
        g_unneuter_state = 2;
    }
}

BOOL CALLBACK find_main_window(HWND window, LPARAM parameter)
{
    if (!IsWindowVisible(window) || GetWindow(window, GW_OWNER) != nullptr)
    {
        return TRUE;
    }
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);
    if (pid != GetCurrentProcessId())
    {
        return TRUE;
    }
    *reinterpret_cast<HWND*>(parameter) = window;
    return FALSE;
}

HWND find_current_window()
{
    HWND window = nullptr;
    EnumWindows(find_main_window, reinterpret_cast<LPARAM>(&window));
    return window;
}

HWND wait_for_main_window()
{
    for (;;)
    {
        const HWND window = find_current_window();
        if (window != nullptr)
        {
            return window;
        }
        Sleep(WINDOW_POLL_INTERVAL_MS);
    }
}

HWND wait_for_new_window()
{
    for (int attempt = 0; attempt < REAPPEAR_ATTEMPTS; ++attempt)
    {
        const HWND window = find_current_window();
        if (window != nullptr)
        {
            return window;
        }
        Sleep(WINDOW_POLL_INTERVAL_MS);
    }
    return nullptr;
}

LRESULT CALLBACK on_window_message(int code, WPARAM wparam, LPARAM lparam)
{
    if (code == HC_ACTION)
    {
        const auto* message = reinterpret_cast<const CWPSTRUCT*>(lparam);
        if (message->hwnd == g_window)
        {
            const bool user_close =
                message->message == WM_CLOSE ||
                (message->message == WM_SYSCOMMAND && (message->wParam & 0xFFF0) == SC_CLOSE);
            if (user_close || message->message == WM_DESTROY)
            {
                if (user_close)
                {
                    restore_self_terminate();
                }
                SetEvent(g_close_event);
            }
        }
    }
    return CallNextHookEx(g_hook, code, wparam, lparam);
}

void hook_window_thread(HWND window, DWORD* hooked_thread)
{
    g_window = window;
    const DWORD thread_id = GetWindowThreadProcessId(window, nullptr);
    if (thread_id == *hooked_thread)
    {
        return;
    }
    if (g_hook != nullptr)
    {
        UnhookWindowsHookEx(g_hook);
    }
    g_hook = SetWindowsHookExW(WH_CALLWNDPROC, on_window_message, nullptr, thread_id);
    *hooked_thread = thread_id;
}

DWORD WINAPI quit_thread(LPVOID)
{
    Sleep(GRACE_PERIOD_MS);
    restore_self_terminate();
    TerminateProcess(GetCurrentProcess(), 0);
    return 0;
}

DWORD WINAPI guard_thread(LPVOID)
{
    DWORD hooked_thread = 0;
    for (;;)
    {
        HWND window = wait_for_main_window();
        hook_window_thread(window, &hooked_thread);
        bool closing = false;

        while (!closing)
        {
            maybe_unneuter();
            if (WaitForSingleObject(g_close_event, GUARD_POLL_INTERVAL_MS) == WAIT_OBJECT_0)
            {
                closing = true;
            }
            else if (!IsWindow(window))
            {
                window = wait_for_new_window();
                if (window == nullptr)
                {
                    closing = true;
                }
                else
                {
                    hook_window_thread(window, &hooked_thread);
                }
            }
        }

        Sleep(GRACE_PERIOD_MS);
        const HWND survivor = find_current_window();
        const bool watched_alive = IsWindow(window) != FALSE;
        if (watched_alive || survivor == nullptr)
        {
            restore_self_terminate();
            TerminateProcess(GetCurrentProcess(), 0);
        }
        g_window = survivor;
        ResetEvent(g_close_event);
    }
}
}

void start_close_guard()
{
    capture_terminate_prologue();
    g_close_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (g_close_event == nullptr)
    {
        return;
    }
    const HANDLE thread = CreateThread(nullptr, 0, guard_thread, nullptr, 0, nullptr);
    if (thread != nullptr)
    {
        CloseHandle(thread);
    }
}

void arm_quit_guard()
{
    if (InterlockedExchange(&g_quit_armed, 1) != 0)
    {
        return;
    }
    const HANDLE thread = CreateThread(nullptr, 0, quit_thread, nullptr, 0, nullptr);
    if (thread != nullptr)
    {
        CloseHandle(thread);
    }
}

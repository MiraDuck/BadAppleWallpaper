#include <windows.h>
#include "win32_helpers.h"

static HWND find_defview_owner(void) {
    HWND hwnd = NULL;
    while ((hwnd = FindWindowEx(NULL, hwnd, "WorkerW", NULL)) != NULL) {
        if (FindWindowEx(hwnd, NULL, "SHELLDLL_DefView", NULL) != NULL) return hwnd;
    }
    HWND progman = FindWindow("Progman", NULL);
    if (progman && FindWindowEx(progman, NULL, "SHELLDLL_DefView", NULL) != NULL) {
        return progman; // Windows 11 24H2+ layout
    }
    return NULL;
}

void* get_workerw(void) {
    HWND progman = FindWindow("Progman", NULL);
    SendMessageTimeout(progman, 0x052C, 0xD, 0x1, SMTO_NORMAL, 1000, NULL);

    for (int i = 0; i < 10; i++) {
        HWND defViewOwner = find_defview_owner();
        if (defViewOwner == progman) break; // 24H2+ confirmed, no point retrying
        if (defViewOwner != NULL) {
            HWND workerw = FindWindowEx(NULL, defViewOwner, "WorkerW", NULL);
            if (workerw != NULL) return (void*)workerw;
        }
        Sleep(100); // Explorer hasn't finished building it yet
    }
    return (void*)progman;
}

// void reparent_to_workerw(void* hwnd_ptr) {
//     HWND hwnd = (HWND)hwnd_ptr;
//     HWND target = (HWND)get_workerw();
//     if (target == NULL || hwnd == NULL) return;

//     LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
//     style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME);
//     style |= WS_CHILD;
//     SetWindowLongPtr(hwnd, GWL_STYLE, style);

//     SetParent(hwnd, target);
//     SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

//     HWND defView = FindWindowEx(target, NULL, "SHELLDLL_DefView", NULL);
//     if (defView != NULL) {
//         SetWindowPos(hwnd, defView, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
//     }
// }
void reparent_to_workerw(void* hwnd_ptr, int screenX, int screenY, int width, int height) {
    HWND hwnd = (HWND)hwnd_ptr;
    HWND target = (HWND)get_workerw();
    if (target == NULL || hwnd == NULL) return;

    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME);
    style |= WS_CHILD;
    SetWindowLongPtr(hwnd, GWL_STYLE, style);

    SetParent(hwnd, target);

    // screenX/screenY are absolute desktop coords; convert to target's own
    // client-relative space, since WorkerW's origin may not be (0,0)
    RECT targetRect;
    GetWindowRect(target, &targetRect);
    int relX = screenX - targetRect.left;
    int relY = screenY - targetRect.top;

    SetWindowPos(hwnd, NULL, relX, relY, width, height, SWP_NOZORDER | SWP_FRAMECHANGED);

    HWND defView = FindWindowEx(target, NULL, "SHELLDLL_DefView", NULL);
    if (defView != NULL) {
        SetWindowPos(hwnd, defView, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void hide_taskbar_icon(void* hwnd_ptr) {
    HWND hwnd = (HWND)hwnd_ptr;
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    exStyle &= ~WS_EX_APPWINDOW;
    exStyle |= WS_EX_TOOLWINDOW;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
    ShowWindow(hwnd, SW_HIDE);
    ShowWindow(hwnd, SW_SHOW);
}

static volatile int escapeHeld = 0;
static HHOOK keyboardHook = NULL;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT*)lParam;
        if (kb->vkCode == VK_ESCAPE) {
            if (wParam == WM_KEYDOWN) escapeHeld = 1;
            else if (wParam == WM_KEYUP) escapeHeld = 0;
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

void start_keyboard_hook(void) {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
}

int is_escape_held(void) {
    return escapeHeld;
}
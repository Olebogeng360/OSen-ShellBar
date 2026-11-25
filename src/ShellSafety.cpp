#include "ShellSafety.h"
#include <shellapi.h>
#include <string>

namespace {
BOOL CALLBACK EnumTaskbars(HWND hwnd, LPARAM value) {
    wchar_t cls[128]{};
    if (!GetClassNameW(hwnd, cls, 128)) return TRUE;
    if (lstrcmpW(cls, L"Shell_TrayWnd") == 0 || lstrcmpW(cls, L"Shell_SecondaryTrayWnd") == 0) {
        auto* states = reinterpret_cast<std::vector<NativeTaskbarState>*>(value);
        states->push_back({hwnd, IsWindowVisible(hwnd) != FALSE});
    }
    return TRUE;
}
}

bool ShellSafety::Discover() {
    taskbars_.clear();
    EnumWindows(EnumTaskbars, reinterpret_cast<LPARAM>(&taskbars_));
    return !taskbars_.empty();
}

bool ShellSafety::LaunchWatchdog(const wchar_t* path, DWORD pid) {
    const std::wstring eventName = L"Local\\CustomShellBar.CleanExit." + std::to_wstring(pid);
    cleanEvent_ = CreateEventW(nullptr, TRUE, FALSE, eventName.c_str());
    if (!cleanEvent_) return false;
    std::wstring command = L"\"" + std::wstring(path) + L"\" --pid " + std::to_wstring(pid) + L" --event \"" + eventName + L"\"";
    STARTUPINFOW si{}; si.cb = sizeof(si);
    return CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &watchdog_) != FALSE;
}

bool ShellSafety::HideAll() {
    if (taskbars_.empty() || !cleanEvent_ || !watchdog_.hProcess) return false;
    if (WaitForSingleObject(watchdog_.hProcess, 0) != WAIT_TIMEOUT) return false;
    for (const auto& state : taskbars_) if (IsWindow(state.hwnd)) ShowWindow(state.hwnd, SW_HIDE);
    hidden_ = true;
    return true;
}

void ShellSafety::RestoreAll() noexcept {
    if (!hidden_) return;
    for (const auto& state : taskbars_) {
        if (IsWindow(state.hwnd) && state.wasVisible) {
            ShowWindow(state.hwnd, SW_SHOW);
            SetWindowPos(state.hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        }
    }
    hidden_ = false;
}

void ShellSafety::MarkCleanShutdown() noexcept {
    RestoreAll();
    if (cleanEvent_) SetEvent(cleanEvent_);
}

ShellSafety::~ShellSafety() {
    MarkCleanShutdown();
    if (watchdog_.hThread) CloseHandle(watchdog_.hThread);
    if (watchdog_.hProcess) CloseHandle(watchdog_.hProcess);
    if (cleanEvent_) CloseHandle(cleanEvent_);
}

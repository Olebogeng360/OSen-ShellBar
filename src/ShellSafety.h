#pragma once
#include <windows.h>
#include <vector>

struct NativeTaskbarState {
    HWND hwnd{};
    bool wasVisible{};
};

class ShellSafety final {
public:
    bool Discover();
    bool HideAll();
    void RestoreAll() noexcept;
    bool LaunchWatchdog(const wchar_t* watchdogPath, DWORD mainPid);
    void MarkCleanShutdown() noexcept;
    ~ShellSafety();
private:
    std::vector<NativeTaskbarState> taskbars_;
    HANDLE cleanEvent_{};
    PROCESS_INFORMATION watchdog_{};
    bool hidden_{};
};

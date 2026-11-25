#pragma once

#include <windows.h>
#include <vector>
#include <string>

struct AppWindow
{
    HWND hwnd = nullptr;

    std::wstring title;

    std::wstring processPath;

    DWORD processId = 0;

    HICON icon = nullptr;
};

class WindowTracker
{
public:
    void Refresh();

    const std::vector<AppWindow>&
    GetWindows() const;

private:
    std::vector<AppWindow> m_windows;
};
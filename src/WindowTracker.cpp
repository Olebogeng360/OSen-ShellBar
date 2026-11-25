#include "WindowTracker.h"

#include <dwmapi.h>
#include <psapi.h>

#pragma comment(lib, "dwmapi.lib")

namespace
{
BOOL CALLBACK EnumWindowsCallback(
    HWND hwnd,
    LPARAM lParam
)
{
    auto* windows =
        reinterpret_cast<
            std::vector<AppWindow>*
        >(lParam);

    if (!IsWindowVisible(hwnd))
        return TRUE;

    LONG_PTR exStyle =
        GetWindowLongPtr(
            hwnd,
            GWL_EXSTYLE
        );

    if (exStyle & WS_EX_TOOLWINDOW)
        return TRUE;

    BOOL cloaked = FALSE;

    DwmGetWindowAttribute(
        hwnd,
        DWMWA_CLOAKED,
        &cloaked,
        sizeof(cloaked)
    );

    if (cloaked)
        return TRUE;

    wchar_t title[256]{};

    GetWindowTextW(
        hwnd,
        title,
        ARRAYSIZE(title)
    );

    if (wcslen(title) == 0)
        return TRUE;

    DWORD pid = 0;

    GetWindowThreadProcessId(
        hwnd,
        &pid
    );

    HANDLE process =
        OpenProcess(
            PROCESS_QUERY_LIMITED_INFORMATION,
            FALSE,
            pid
        );

    wchar_t path[MAX_PATH]{};

    if (process)
    {
        DWORD size =
            ARRAYSIZE(path);

        QueryFullProcessImageNameW(
            process,
            0,
            path,
            &size
        );

        CloseHandle(process);
    }

    AppWindow app;

    app.hwnd = hwnd;
    app.title = title;
    app.processPath = path;
    app.processId = pid;

    app.icon =
        reinterpret_cast<HICON>(
            SendMessage(
                hwnd,
                WM_GETICON,
                ICON_BIG,
                0
            )
        );

    if (!app.icon)
    {
        app.icon =
            reinterpret_cast<HICON>(
                GetClassLongPtr(
                    hwnd,
                    GCLP_HICON
                )
            );
    }

    windows->push_back(
        std::move(app)
    );

    return TRUE;
}
}

void WindowTracker::Refresh()
{
    m_windows.clear();

    EnumWindows(
        EnumWindowsCallback,
        reinterpret_cast<LPARAM>(
            &m_windows
        )
    );
}

const std::vector<AppWindow>&
WindowTracker::GetWindows() const
{
    return m_windows;
}
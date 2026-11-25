#include <windows.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <string>
#include "ShellSafety.h"
#include "AppBarManager.h"
#include "WindowTracker.h"
#include <windowsx.h>
#include "ShellRenderer.h"

#pragma comment(lib, "dwmapi.lib")

namespace
{
    constexpr COLORREF kAccent = RGB(255, 122, 47);
    constexpr COLORREF kSurface = RGB(20, 20, 20);
    constexpr COLORREF kText = RGB(255, 255, 255);

    constexpr UINT_PTR kClockTimer = 1;
    constexpr UINT_PTR kWindowRefreshTimer = 2;
    constexpr UINT_PTR kAutoHideTimer = 3;

    constexpr DWORD kAutoHideDelayMs = 350;

    constexpr int kVisibleReserveHeight = 84;
    constexpr int kHiddenReserveHeight = 3;
    constexpr int kRevealZoneHeight = 8;
    constexpr int kHiddenVisiblePixels = 1;

    constexpr UINT_PTR kAnimationTimer = 4;

    constexpr UINT kAnimationIntervalMs = 16;
    constexpr DWORD kRevealHoldMs = 600;

    constexpr int kAnimationStep = 12;

    constexpr int kEmergencyHotkey = 0x4342;

    constexpr wchar_t kAppClass[] =
        L"CustomShellBar.AppCapsule";

    constexpr wchar_t kTrayClass[] =
        L"CustomShellBar.SystemCapsule";

    constexpr wchar_t kStartClass[] =
        L"CustomShellBar.StartPanel";

    ShellSafety gSafety;
    AppBarManager gAppBar;
    WindowTracker gWindowTracker;
    ShellRenderer gRenderer;

    HWND gApp{};
    HWND gTray{};
    HWND gStart{};

    bool gStartOpen = false;

    bool gBarsVisible = true;
    DWORD gPointerLeftBarsAt = 0;

    bool gAnimating = false;

    int gCurrentAppY = 0;
    int gCurrentTrayY = 0;

    int gTargetAppY = 0;
    int gTargetTrayY = 0;

   DWORD gLastRevealAt = 0;

    void RoundedWindow(
        HWND window,
        int radius
    )
    {
        if (!window)
        {
            return;
        }

        RECT clientRect{};

        if (!GetClientRect(window, &clientRect))
        {
            return;
        }

        HRGN region = CreateRoundRectRgn(
            0,
            0,
            clientRect.right + 1,
            clientRect.bottom + 1,
            radius,
            radius
        );

        if (!region)
        {
            return;
        }

        // Windows owns the region after a successful SetWindowRgn.
        if (!SetWindowRgn(window, region, TRUE))
        {
            DeleteObject(region);
        }
    }


    bool PointInsideWindow(
    HWND window,
    POINT point
    )

    {
    if (!IsWindow(window))
    {
        return false;
    }

    RECT windowRect{};

    if (!GetWindowRect(
        window,
        &windowRect
    ))
    {
        return false;
    }

    return PtInRect(
        &windowRect,
        point
    ) != FALSE;
    }

    void PlaceWindows()
    {
    HMONITOR monitor =
        MonitorFromWindow(
            gApp,
            MONITOR_DEFAULTTOPRIMARY
        );

    MONITORINFO monitorInfo{};
    monitorInfo.cbSize =
        sizeof(monitorInfo);

    if (!GetMonitorInfoW(
        monitor,
        &monitorInfo
    ))
    {
        return;
    }

    const RECT& monitorRect =
        monitorInfo.rcMonitor;

    constexpr int appWidth = 700;
    constexpr int appHeight = 72;

    constexpr int trayWidth = 330;
    constexpr int trayHeight = 64;

    constexpr int edgeGap = 18;
    constexpr int bottomGap = 12;

    constexpr int startWidth = 520;
    constexpr int startHeight = 630;
    constexpr int startGap = 8;

    const int appX =
        monitorRect.left +
        (
            monitorRect.right -
            monitorRect.left -
            appWidth
        ) / 2;

    const int trayX =
        monitorRect.right -
        trayWidth -
        edgeGap;

    int appY{};
    int trayY{};

    if (gBarsVisible)
    {
        appY =
            monitorRect.bottom -
            appHeight -
            bottomGap;

        trayY =
            monitorRect.bottom -
            trayHeight -
            bottomGap;
    }
    else
    {
        // Leave a tiny visible strip so the hidden state
        // is visually discoverable.
        appY =
            monitorRect.bottom -
            kHiddenVisiblePixels;

        trayY =
            monitorRect.bottom -
            kHiddenVisiblePixels;
    }

    SetWindowPos(
        gApp,
        HWND_TOPMOST,
        appX,
        appY,
        appWidth,
        appHeight,
        SWP_NOACTIVATE |
            SWP_SHOWWINDOW
    );

    SetWindowPos(
        gTray,
        HWND_TOPMOST,
        trayX,
        trayY,
        trayWidth,
        trayHeight,
        SWP_NOACTIVATE |
            SWP_SHOWWINDOW
    );

    RECT appRect{};
    GetWindowRect(
        gApp,
        &appRect
    );

    UINT startFlags =
        SWP_NOACTIVATE;

    if (
        gStartOpen &&
        gBarsVisible
    )
    {
        startFlags |=
            SWP_SHOWWINDOW;
    }
    else
    {
        startFlags |=
            SWP_HIDEWINDOW;
    }

    SetWindowPos(
        gStart,
        HWND_TOPMOST,
        appRect.left,
        appRect.top -
            startHeight -
            startGap,
        startWidth,
        startHeight,
        startFlags
    );

    RoundedWindow(
        gApp,
        44
    );

    RoundedWindow(
        gTray,
        40
    );

    RoundedWindow(
        gStart,
        40
    );
    }

    void ShowShellBars()
    {
        if (gBarsVisible)
        {
            return;
        }

        gBarsVisible = true;
        gPointerLeftBarsAt = 0;

    PlaceWindows();

    InvalidateRect(
        gApp,
        nullptr,
        FALSE
    );

    InvalidateRect(
        gTray,
        nullptr,
        FALSE
    );
    }

    void HideShellBars()
    {
        if (
            !gBarsVisible ||
            gStartOpen
        )
        {
            return;
        }

        gBarsVisible = false;
        gPointerLeftBarsAt = 0;

        PlaceWindows();
    }

    void UpdateAutoHide()
    {
        POINT pointer{};

        if (!GetCursorPos(&pointer))
        {
            return;
        }

        HMONITOR monitor =
        MonitorFromWindow(
            gApp,
            MONITOR_DEFAULTTOPRIMARY
        );

        MONITORINFO monitorInfo{};
        monitorInfo.cbSize =
        sizeof(monitorInfo);

        if (!GetMonitorInfoW(
            monitor,
            &monitorInfo
        ))
        {
            return;
        }

        const RECT& monitorRect =
        monitorInfo.rcMonitor;

        const bool pointerInRevealZone =
            pointer.x >= monitorRect.left &&
            pointer.x < monitorRect.right &&
            pointer.y >=
            monitorRect.bottom -
            kRevealZoneHeight &&
            pointer.y < monitorRect.bottom;

        const bool pointerOverApp =
        PointInsideWindow(
            gApp,
            pointer
        );

        const bool pointerOverTray =
            PointInsideWindow(
               gTray,
               pointer
           );

        const bool pointerOverStart =
        gStartOpen &&
        IsWindowVisible(gStart) &&
        PointInsideWindow(
            gStart,
            pointer
        );

        if (!gBarsVisible)
        {
            if (pointerInRevealZone)
            {
                ShowShellBars();
            }

            return;
        }

        if (
            pointerOverApp ||
            pointerOverTray ||
            pointerOverStart ||
            gStartOpen
        )

        {
            gPointerLeftBarsAt = 0;
            return;
        }

        if (gPointerLeftBarsAt == 0)
        {
            gPointerLeftBarsAt =
            GetTickCount();

            return;
        }

        const DWORD elapsed =
        GetTickCount() -
        gPointerLeftBarsAt;

        if (elapsed >= kAutoHideDelayMs)
        {
            HideShellBars();
        }
    }

    void ShutDown()
{
    if (IsWindow(gTray))
    {
        KillTimer(
            gTray,
            kClockTimer
        );
    }

    if (IsWindow(gApp))
    {
        KillTimer(
            gApp,
            kWindowRefreshTimer
        );

        KillTimer(
            gApp,
            kAutoHideTimer
        );
    }

    gAppBar.Unregister();
    gSafety.MarkCleanShutdown();

    if (IsWindow(gStart))
    {
        DestroyWindow(gStart);
        gStart = nullptr;
    }

    if (IsWindow(gTray))
    {
        DestroyWindow(gTray);
        gTray = nullptr;
    }

    if (IsWindow(gApp))
    {
        DestroyWindow(gApp);
        gApp = nullptr;
    }
}

    LRESULT CALLBACK Proc(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    )
    {
        switch (message)
        {
        case WM_CREATE:
        {
            BOOL transitionsDisabled = FALSE;

            DwmSetWindowAttribute(
                window,
                DWMWA_TRANSITIONS_FORCEDISABLED,
                &transitionsDisabled,
                sizeof(transitionsDisabled)
            );

            return 0;
        }

        case WM_PAINT:
{
    PAINTSTRUCT paintStruct{};

    HDC deviceContext =
        BeginPaint(
            window,
            &paintStruct
        );

    if (window == gApp)
    {
        gRenderer.DrawApplicationCapsule(
            window,
            deviceContext,
            gWindowTracker,
            gStartOpen
        );
    }
    else if (window == gTray)
    {
        gRenderer.DrawSystemCapsule(
            window,
            deviceContext
        );
    }
    else if (window == gStart)
    {
        gRenderer.DrawStartPanel(
            window,
            deviceContext
        );
    }

    EndPaint(
        window,
        &paintStruct
    );

    return 0;
}

        case WM_LBUTTONUP:
{
    const int x =
        GET_X_LPARAM(lParam);

    const int y =
        GET_Y_LPARAM(lParam);

    if (window == gApp)
    {
        gPointerLeftBarsAt = 0;

        constexpr int startButtonEndX = 82;
        constexpr int buttonWidth = 56;
        constexpr int buttonGap = 8;
        constexpr int firstAppX = 82;
        constexpr int buttonStride =
            buttonWidth + buttonGap;

        // Start button.
        if (x < startButtonEndX)
        {
            gBarsVisible = true;

            gStartOpen =
                !gStartOpen;

            gPointerLeftBarsAt = 0;


            PlaceWindows();

            return 0;
        }

        if (x < firstAppX)
        {
            return 0;
        }

        const int relativeX =
            x - firstAppX;

        // Ignore clicks in the gap between app buttons.
        if (
            relativeX % buttonStride >=
            buttonWidth
        )
        {
            return 0;
        }

        const int index =
            relativeX /
            buttonStride;

        const auto& applications =
            gWindowTracker.GetWindows();

        if (
            index < 0 ||
            index >=
                static_cast<int>(
                    applications.size()
                )
        )
        {
            return 0;
        }

        const AppWindow& application =
            applications[
                static_cast<size_t>(
                    index
                )
            ];

        if (!IsWindow(application.hwnd))
        {
            return 0;
        }

        if (
            GetForegroundWindow() ==
            application.hwnd
        )
        {
            ShowWindow(
                application.hwnd,
                SW_MINIMIZE
            );
        }
        else
        {
            if (IsIconic(application.hwnd))
            {
                ShowWindow(
                    application.hwnd,
                    SW_RESTORE
                );
            }

            SetForegroundWindow(
                application.hwnd
            );
        }

        InvalidateRect(
            gApp,
            nullptr,
            FALSE
        );

        return 0;
    }

    if (
        window == gStart &&
        y > 540
    )
    {
        PostMessageW(
            gApp,
            WM_CLOSE,
            0,
            0
        );

        return 0;
    }

    return 0;
}

        case WM_RBUTTONUP:
        {
            HMENU menu =
                CreatePopupMenu();

            AppendMenuW(
                menu,
                MF_STRING,
                1,
                L"Settings (coming later)"
            );

            AppendMenuW(
                menu,
                MF_STRING,
                2,
                L"Exit and restore Windows taskbar"
            );

            POINT cursor{};
            GetCursorPos(&cursor);

            int command =
                TrackPopupMenu(
                    menu,
                    TPM_RETURNCMD |
                        TPM_RIGHTBUTTON,
                    cursor.x,
                    cursor.y,
                    0,
                    window,
                    nullptr
                );

            DestroyMenu(menu);

            if (command == 2)
            {
                PostMessageW(
                    gApp,
                    WM_CLOSE,
                    0,
                    0
                );
            }

            return 0;
        }

        case WM_TIMER:
{
    if (
        window == gTray &&
        wParam == kClockTimer
    )
    {
        InvalidateRect(
            gTray,
            nullptr,
            FALSE
        );
    }
    else if (
        window == gApp &&
        wParam == kWindowRefreshTimer
    )
    {
        gWindowTracker.Refresh();

        InvalidateRect(
            gApp,
            nullptr,
            FALSE
        );
    }
    else if (
        window == gApp &&
        wParam == kAutoHideTimer
    )
    {
        UpdateAutoHide();
    }

    return 0;
}

        case WM_HOTKEY:
            if (
                static_cast<int>(wParam) ==
                kEmergencyHotkey
            )
            {
                PostMessageW(
                    gApp,
                    WM_CLOSE,
                    0,
                    0
                );
            }

            return 0;

        case WM_DISPLAYCHANGE:
            PlaceWindows();
            return 0;

        case WM_DPICHANGED:
        {
            const RECT* suggestedRect =
                reinterpret_cast<const RECT*>(
                    lParam
                );

            if (suggestedRect)
            {
                SetWindowPos(
                    window,
                    nullptr,
                    suggestedRect->left,
                    suggestedRect->top,
                    suggestedRect->right -
                        suggestedRect->left,
                    suggestedRect->bottom -
                        suggestedRect->top,
                    SWP_NOZORDER |
                        SWP_NOACTIVATE
                );
            }

            PlaceWindows();
            return 0;
        }

        case WM_CLOSE:
            if (window == gApp)
            {
                ShutDown();
            }
            else
            {
                ShowWindow(
                    window,
                    SW_HIDE
                );
            }

            return 0;

        case WM_DESTROY:
            if (window == gApp)
            {
                PostQuitMessage(0);
            }

            return 0;
        }

        return DefWindowProcW(
            window,
            message,
            wParam,
            lParam
        );
    }

    bool Register(
        HINSTANCE instance,
        const wchar_t* className
    )
    {
        WNDCLASSEXW windowClass{};
        windowClass.cbSize =
            sizeof(windowClass);

        windowClass.hInstance =
            instance;

        windowClass.lpfnWndProc =
            Proc;

        windowClass.hCursor =
            LoadCursorW(
                nullptr,
                IDC_ARROW
            );

        windowClass.lpszClassName =
            className;

        return RegisterClassExW(
            &windowClass
        ) != 0;
    }
}

int WINAPI wWinMain(HINSTANCE h,HINSTANCE,PWSTR cmd,int){
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    HANDLE mutex = CreateMutexW(
    nullptr,
    TRUE,
    L"Local\\CustomShellBar.Singleton"
);

DWORD mutexError = GetLastError();

if (!mutex)
{
    MessageBoxW(
        nullptr,
        L"Failed to create mutex.",
        L"CustomShellBar",
        MB_OK | MB_ICONERROR
    );

    return 2;
}

if (mutexError == ERROR_ALREADY_EXISTS)
{
    MessageBoxW(
        nullptr,
        L"CustomShellBar is already running.",
        L"CustomShellBar",
        MB_OK | MB_ICONINFORMATION
    );

    return 2;
}
    bool hide=wcsstr(cmd,L"--hide-native")!=nullptr;
    if(!Register(h,kAppClass)||!Register(h,kTrayClass)||!Register(h,kStartClass))
{
    MessageBoxW(
        nullptr,
        L"Window class registration failed.",
        L"CustomShellBar",
        MB_OK | MB_ICONERROR
    );
    return 3;
}
    DWORD style=WS_POPUP;DWORD ex=WS_EX_TOOLWINDOW|WS_EX_TOPMOST;
    gApp=CreateWindowExW(ex,kAppClass,L"CustomShellBar",style,0,0,700,72,nullptr,nullptr,h,nullptr);
    gTray=CreateWindowExW(ex,kTrayClass,L"CustomShellBar System",style,0,0,330,64,nullptr,nullptr,h,nullptr);
    gStart=CreateWindowExW(ex,kStartClass,L"CustomShellBar Start",style,0,0,520,630,nullptr,nullptr,h,nullptr);

if(!gApp||!gTray||!gStart)
{
    wchar_t msg[256];
    swprintf_s(
        msg,
        L"Window creation failed. GetLastError=%lu",
        GetLastError()
    );

    MessageBoxW(
        nullptr,
        msg,
        L"CustomShellBar",
        MB_OK | MB_ICONERROR
    );

    return 4;
}



if (
    !RegisterHotKey(
        gApp,
        kEmergencyHotkey,
        MOD_CONTROL |
            MOD_ALT |
            MOD_SHIFT |
            MOD_NOREPEAT,
        VK_F12
    )
)

{
    MessageBoxW(
        nullptr,
        L"The emergency shortcut could not be registered. "
        L"The native Windows taskbar will not be hidden.",
        L"CustomShellBar",
        MB_OK | MB_ICONERROR
    );

    ShutDown();
    CloseHandle(mutex);

    return 5;
}
    
if (
    SetTimer(
        gTray,
        kClockTimer,
        1000,
        nullptr
    ) == 0
)

{
    MessageBoxW(
        nullptr,
        L"Failed to start the clock timer.",
        L"CustomShellBar",
        MB_OK | MB_ICONWARNING
    );
}

if (
    SetTimer(
        gApp,
        kAutoHideTimer,
        50,
        nullptr
    ) == 0
)

{
    MessageBoxW(
        nullptr,
        L"Failed to start taskbar auto-hide.",
        L"CustomShellBar",
        MB_OK | MB_ICONWARNING
    );
}

if (
    SetTimer(
        gApp,
        kWindowRefreshTimer,
        1000,
        nullptr
    ) == 0
)

{
    MessageBoxW(
        nullptr,
        L"Failed to start application detection.",
        L"CustomShellBar",
        MB_OK | MB_ICONWARNING
    );
}

    PlaceWindows();

ShowWindow(
    gApp,
    SW_SHOW
);

ShowWindow(
    gTray,
    SW_SHOW
);

if (gAppBar.Register(gApp))
{
    gAppBar.SetBottomPosition(
    gApp,
    kVisibleReserveHeight
    );

    // The AppBar changed SPI_GETWORKAREA.
    PlaceWindows();
}

else
{
    MessageBoxW(
        nullptr,
        L"Could not reserve desktop space. "
        L"CustomShellBar will continue without AppBar reservation.",
        L"CustomShellBar",
        MB_OK | MB_ICONWARNING
    );
}

    gWindowTracker.Refresh();

    for (const auto& app : gWindowTracker.GetWindows())
    {
        OutputDebugStringW(app.title.c_str());
        OutputDebugStringW(L"\n");
    }

    UpdateWindow(gApp);UpdateWindow(gTray);
    if(hide){
        wchar_t exe[MAX_PATH]{};GetModuleFileNameW(nullptr,exe,MAX_PATH);std::wstring path=exe;auto pos=path.find_last_of(L"\\/");path=path.substr(0,pos+1)+L"CustomShellBarWatchdog.exe";
        if(gSafety.Discover()&&gSafety.LaunchWatchdog(path.c_str(),GetCurrentProcessId()))gSafety.HideAll();
        else MessageBoxW(nullptr,L"Safety startup failed. The native taskbar will remain visible.",L"CustomShellBar",MB_ICONWARNING);
    }
    MSG msg{};while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);} UnregisterHotKey(gApp,kEmergencyHotkey);CloseHandle(mutex);return 0;
}

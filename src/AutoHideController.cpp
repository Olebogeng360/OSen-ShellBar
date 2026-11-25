#include "AutoHideController.h"

void AutoHideController::Initialize(
    HWND appWindow,
    HWND trayWindow,
    HWND startWindow
)
{
    appWindow_ = appWindow;
    trayWindow_ = trayWindow;
    startWindow_ = startWindow;

    barsVisible_ = true;
    startPanelOpen_ = false;

    pointerLeftAt_ = 0;
    lastRevealAt_ = GetTickCount();

    PlaceWindows();
}

void AutoHideController::SetStartPanelOpen(
    bool open
)
{
    startPanelOpen_ = open;

    if (open)
    {
        barsVisible_ = true;
        lastRevealAt_ = GetTickCount();
    }

    pointerLeftAt_ = 0;

    PlaceWindows();

    InvalidateRect(
        appWindow_,
        nullptr,
        FALSE
    );
}

void AutoHideController::NotifyInteraction()
{
    pointerLeftAt_ = 0;
    lastRevealAt_ = GetTickCount();

    if (!barsVisible_)
    {
        ShowBars();
    }
}

bool AutoHideController::AreBarsVisible() const noexcept
{
    return barsVisible_;
}

bool AutoHideController::IsStartPanelOpen() const noexcept
{
    return startPanelOpen_;
}

bool AutoHideController::GetMonitorRect(
    RECT& monitorRect
) const
{
    if (!IsWindow(appWindow_))
    {
        return false;
    }

    HMONITOR monitor =
        MonitorFromWindow(
            appWindow_,
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
        return false;
    }

    monitorRect =
        monitorInfo.rcMonitor;

    return true;
}

bool AutoHideController::PointInsideWindow(
    HWND window,
    POINT point
)
{
    if (
        !IsWindow(window) ||
        !IsWindowVisible(window)
    )
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

void AutoHideController::PlaceWindows()
{
    RECT monitorRect{};

    if (!GetMonitorRect(monitorRect))
    {
        return;
    }

    const int appX =
        monitorRect.left +
        (
            monitorRect.right -
            monitorRect.left -
            AppWidth
        ) / 2;

    const int trayX =
        monitorRect.right -
        TrayWidth -
        EdgeGap;

    const int appY =
        barsVisible_
            ? monitorRect.bottom -
                AppHeight -
                BottomGap
            : monitorRect.bottom -
                HiddenVisiblePixels;

    const int trayY =
        barsVisible_
            ? monitorRect.bottom -
                TrayHeight -
                BottomGap
            : monitorRect.bottom -
                HiddenVisiblePixels;

    SetWindowPos(
        appWindow_,
        HWND_TOPMOST,
        appX,
        appY,
        AppWidth,
        AppHeight,
        SWP_NOACTIVATE |
            SWP_SHOWWINDOW
    );

    SetWindowPos(
        trayWindow_,
        HWND_TOPMOST,
        trayX,
        trayY,
        TrayWidth,
        TrayHeight,
        SWP_NOACTIVATE |
            SWP_SHOWWINDOW
    );

    const bool showStart =
        startPanelOpen_ &&
        barsVisible_;

    UINT startFlags =
        SWP_NOACTIVATE |
        (
            showStart
                ? SWP_SHOWWINDOW
                : SWP_HIDEWINDOW
        );

    SetWindowPos(
        startWindow_,
        HWND_TOPMOST,
        appX,
        appY -
            StartHeight -
            StartGap,
        StartWidth,
        StartHeight,
        startFlags
    );
}

void AutoHideController::ShowBars()
{
    if (barsVisible_)
    {
        return;
    }

    barsVisible_ = true;
    pointerLeftAt_ = 0;
    lastRevealAt_ = GetTickCount();

    PlaceWindows();

    InvalidateRect(
        appWindow_,
        nullptr,
        FALSE
    );

    InvalidateRect(
        trayWindow_,
        nullptr,
        FALSE
    );
}

void AutoHideController::HideBars()
{
    if (
        !barsVisible_ ||
        startPanelOpen_
    )
    {
        return;
    }

    barsVisible_ = false;
    pointerLeftAt_ = 0;

    PlaceWindows();
}

void AutoHideController::Update()
{
    POINT pointer{};

    if (!GetCursorPos(&pointer))
    {
        return;
    }

    RECT monitorRect{};

    if (!GetMonitorRect(monitorRect))
    {
        return;
    }

    const bool pointerInRevealZone =
        pointer.x >= monitorRect.left &&
        pointer.x < monitorRect.right &&
        pointer.y >=
            monitorRect.bottom -
            RevealZoneHeight &&
        pointer.y < monitorRect.bottom;

    const bool pointerOverApp =
        PointInsideWindow(
            appWindow_,
            pointer
        );

    const bool pointerOverTray =
        PointInsideWindow(
            trayWindow_,
            pointer
        );

    const bool pointerOverStart =
        startPanelOpen_ &&
        PointInsideWindow(
            startWindow_,
            pointer
        );

    if (!barsVisible_)
    {
        if (pointerInRevealZone)
        {
            ShowBars();
        }

        return;
    }

    const DWORD now =
        GetTickCount();

    const bool revealHoldActive =
        now - lastRevealAt_ <
        RevealHoldMs;

    if (
        pointerInRevealZone ||
        pointerOverApp ||
        pointerOverTray ||
        pointerOverStart ||
        startPanelOpen_ ||
        revealHoldActive
    )
    {
        pointerLeftAt_ = 0;
        return;
    }

    if (pointerLeftAt_ == 0)
    {
        pointerLeftAt_ = now;
        return;
    }

    if (
        now - pointerLeftAt_ >=
        AutoHideDelayMs
    )
    {
        HideBars();
    }
}
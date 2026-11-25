#include "AutoHideController.h"

#include <algorithm>
#include <cstdlib>

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
    animating_ = false;
    geometryInitialized_ = false;

    pointerLeftAt_ = 0;
    lastRevealAt_ = GetTickCount();

    RecalculateGeometry();

    currentAppY_ = visibleAppY_;
    currentTrayY_ = visibleTrayY_;

    targetAppY_ = visibleAppY_;
    targetTrayY_ = visibleTrayY_;

    geometryInitialized_ = true;

    ApplyCurrentPositions();
    UpdateStartPanelPosition();
}

void AutoHideController::SetStartPanelOpen(
    bool open
)
{
    startPanelOpen_ = open;
    pointerLeftAt_ = 0;

    if (open)
    {
        lastRevealAt_ = GetTickCount();

        if (!barsVisible_)
        {
            BeginShowAnimation();
        }
    }

    UpdateStartPanelPosition();

    if (IsWindow(appWindow_))
    {
        InvalidateRect(
            appWindow_,
            nullptr,
            FALSE
        );
    }
}

void AutoHideController::NotifyInteraction()
{
    pointerLeftAt_ = 0;
    lastRevealAt_ = GetTickCount();

    if (!barsVisible_)
    {
        BeginShowAnimation();
    }
}

void AutoHideController::HandleDisplayChange()
{
    RecalculateGeometry();

    if (!geometryInitialized_)
    {
        currentAppY_ =
            barsVisible_
                ? visibleAppY_
                : hiddenAppY_;

        currentTrayY_ =
            barsVisible_
                ? visibleTrayY_
                : hiddenTrayY_;

        geometryInitialized_ = true;
    }

    targetAppY_ =
        barsVisible_
            ? visibleAppY_
            : hiddenAppY_;

    targetTrayY_ =
        barsVisible_
            ? visibleTrayY_
            : hiddenTrayY_;

    if (!animating_)
    {
        currentAppY_ = targetAppY_;
        currentTrayY_ = targetTrayY_;
    }

    ApplyCurrentPositions();
    UpdateStartPanelPosition();
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

int AutoHideController::MoveTowards(
    int current,
    int target
)
{
    const int distance =
        target - current;

    if (distance == 0)
    {
        return target;
    }

    int step =
        static_cast<int>(
            static_cast<double>(
                std::abs(distance)
            ) *
            AnimationEase
        );

    step = std::max(
        step,
        MinimumAnimationStep
    );

    if (std::abs(distance) <= step)
    {
        return target;
    }

    return current +
        (
            distance > 0
                ? step
                : -step
        );
}

void AutoHideController::RecalculateGeometry()
{
    RECT monitorRect{};

    if (!GetMonitorRect(monitorRect))
    {
        return;
    }

    appX_ =
        monitorRect.left +
        (
            monitorRect.right -
            monitorRect.left -
            AppWidth
        ) / 2;

    trayX_ =
        monitorRect.right -
        TrayWidth -
        EdgeGap;

    visibleAppY_ =
        monitorRect.bottom -
        AppHeight -
        BottomGap;

    visibleTrayY_ =
        monitorRect.bottom -
        TrayHeight -
        BottomGap;

    hiddenAppY_ =
        monitorRect.bottom -
        HiddenVisiblePixels;

    hiddenTrayY_ =
        monitorRect.bottom -
        HiddenVisiblePixels;
}

void AutoHideController::ApplyCurrentPositions()
{
    if (IsWindow(appWindow_))
    {
        SetWindowPos(
            appWindow_,
            HWND_TOPMOST,
            appX_,
            currentAppY_,
            AppWidth,
            AppHeight,
            SWP_NOACTIVATE |
                SWP_SHOWWINDOW
        );
    }

    if (IsWindow(trayWindow_))
    {
        SetWindowPos(
            trayWindow_,
            HWND_TOPMOST,
            trayX_,
            currentTrayY_,
            TrayWidth,
            TrayHeight,
            SWP_NOACTIVATE |
                SWP_SHOWWINDOW
        );
    }
}

void AutoHideController::UpdateStartPanelPosition()
{
    if (!IsWindow(startWindow_))
    {
        return;
    }

    const bool readyToShow =
        startPanelOpen_ &&
        barsVisible_ &&
        !animating_ &&
        currentAppY_ == visibleAppY_;

    const UINT flags =
        SWP_NOACTIVATE |
        (
            readyToShow
                ? SWP_SHOWWINDOW
                : SWP_HIDEWINDOW
        );

    SetWindowPos(
        startWindow_,
        HWND_TOPMOST,
        appX_,
        currentAppY_ -
            StartHeight -
            StartGap,
        StartWidth,
        StartHeight,
        flags
    );
}

void AutoHideController::BeginShowAnimation()
{
    RecalculateGeometry();

    barsVisible_ = true;
    animating_ = true;

    pointerLeftAt_ = 0;
    lastRevealAt_ = GetTickCount();

    targetAppY_ = visibleAppY_;
    targetTrayY_ = visibleTrayY_;

    UpdateStartPanelPosition();
}

void AutoHideController::BeginHideAnimation()
{
    if (startPanelOpen_)
    {
        return;
    }

    RecalculateGeometry();

    barsVisible_ = false;
    animating_ = true;

    pointerLeftAt_ = 0;

    targetAppY_ = hiddenAppY_;
    targetTrayY_ = hiddenTrayY_;

    ShowWindow(
        startWindow_,
        SW_HIDE
    );
}

void AutoHideController::UpdateAnimation()
{
    if (!animating_)
    {
        return;
    }

    currentAppY_ =
        MoveTowards(
            currentAppY_,
            targetAppY_
        );

    currentTrayY_ =
        MoveTowards(
            currentTrayY_,
            targetTrayY_
        );

    ApplyCurrentPositions();

    const bool appFinished =
        currentAppY_ ==
        targetAppY_;

    const bool trayFinished =
        currentTrayY_ ==
        targetTrayY_;

    if (
        appFinished &&
        trayFinished
    )
    {
        animating_ = false;

        UpdateStartPanelPosition();

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
}

void AutoHideController::Update()
{
    // Update animation on every 16 ms timer tick.
    UpdateAnimation();

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

    const DWORD now =
        GetTickCount();

    if (!barsVisible_)
    {
        if (pointerInRevealZone)
        {
            BeginShowAnimation();
        }

        return;
    }

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

    // Do not start a hide transition while the show transition
    // is still running.
    if (animating_)
    {
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
        BeginHideAnimation();
    }
}
#pragma once

#include <windows.h>

class AutoHideController final
{
public:
    void Initialize(
        HWND appWindow,
        HWND trayWindow,
        HWND startWindow
    );

    void Update();

    void SetStartPanelOpen(
        bool open
    );

    void NotifyInteraction();

    [[nodiscard]]
    bool AreBarsVisible() const noexcept;

    [[nodiscard]]
    bool IsStartPanelOpen() const noexcept;

private:
    static constexpr DWORD AutoHideDelayMs = 350;
    static constexpr DWORD RevealHoldMs = 600;

    static constexpr int RevealZoneHeight = 8;
    static constexpr int HiddenVisiblePixels = 1;

    static constexpr int AppWidth = 700;
    static constexpr int AppHeight = 72;

    static constexpr int TrayWidth = 330;
    static constexpr int TrayHeight = 64;

    static constexpr int EdgeGap = 18;
    static constexpr int BottomGap = 12;

    static constexpr int StartWidth = 520;
    static constexpr int StartHeight = 630;
    static constexpr int StartGap = 8;

    HWND appWindow_{};
    HWND trayWindow_{};
    HWND startWindow_{};

    bool barsVisible_ = true;
    bool startPanelOpen_ = false;

    DWORD pointerLeftAt_ = 0;
    DWORD lastRevealAt_ = 0;

    [[nodiscard]]
    bool GetMonitorRect(
        RECT& monitorRect
    ) const;

    [[nodiscard]]
    static bool PointInsideWindow(
        HWND window,
        POINT point
    );

    void PlaceWindows();
    void ShowBars();
    void HideBars();
};
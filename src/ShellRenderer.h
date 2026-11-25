#pragma once

#include <windows.h>

class WindowTracker;

class ShellRenderer final
{
public:
    void DrawApplicationCapsule(
        HWND window,
        HDC deviceContext,
        const WindowTracker& windowTracker,
        bool startPanelOpen
    ) const;

    void DrawSystemCapsule(
        HWND window,
        HDC deviceContext
    ) const;

    void DrawStartPanel(
        HWND window,
        HDC deviceContext
    ) const;

private:
    static constexpr COLORREF AccentColor =
        RGB(255, 122, 47);

    static constexpr COLORREF SurfaceColor =
        RGB(20, 20, 20);

    static constexpr COLORREF TextColor =
        RGB(255, 255, 255);

    HFONT CreateTextFont(
        int size,
        int weight = FW_NORMAL
    ) const;

    void PaintSurface(
        HWND window,
        HDC deviceContext,
        int radius
    ) const;

    void DrawCenteredText(
        HDC deviceContext,
        RECT bounds,
        const wchar_t* text,
        int size
    ) const;

    void DrawStartButton(
        HDC deviceContext,
        const RECT& bounds,
        bool startPanelOpen
    ) const;
};
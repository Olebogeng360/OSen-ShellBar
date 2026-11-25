#include "ShellRenderer.h"
#include "WindowTracker.h"

#include <algorithm>

HFONT ShellRenderer::CreateTextFont(
    int size,
    int weight
) const
{
    return CreateFontW(
        -size,
        0,
        0,
        0,
        weight,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH,
        L"Segoe UI"
    );
}

void ShellRenderer::PaintSurface(
    HWND window,
    HDC deviceContext,
    int radius
) const
{
    RECT clientRect{};

    if (!GetClientRect(
        window,
        &clientRect
    ))
    {
        return;
    }

    HBRUSH fillBrush =
        CreateSolidBrush(
            SurfaceColor
        );

    HPEN borderPen =
        CreatePen(
            PS_SOLID,
            1,
            AccentColor
        );

    if (
        !fillBrush ||
        !borderPen
    )
    {
        if (fillBrush)
        {
            DeleteObject(fillBrush);
        }

        if (borderPen)
        {
            DeleteObject(borderPen);
        }

        return;
    }

    HGDIOBJ oldBrush =
        SelectObject(
            deviceContext,
            fillBrush
        );

    HGDIOBJ oldPen =
        SelectObject(
            deviceContext,
            borderPen
        );

    RoundRect(
        deviceContext,
        0,
        0,
        clientRect.right,
        clientRect.bottom,
        radius,
        radius
    );

    SelectObject(
        deviceContext,
        oldPen
    );

    SelectObject(
        deviceContext,
        oldBrush
    );

    DeleteObject(borderPen);
    DeleteObject(fillBrush);

    SetBkMode(
        deviceContext,
        TRANSPARENT
    );

    SetTextColor(
        deviceContext,
        TextColor
    );
}

void ShellRenderer::DrawCenteredText(
    HDC deviceContext,
    RECT bounds,
    const wchar_t* text,
    int size
) const
{
    HFONT font =
        CreateTextFont(
            size,
            FW_SEMIBOLD
        );

    if (!font)
    {
        return;
    }

    HGDIOBJ oldFont =
        SelectObject(
            deviceContext,
            font
        );

    DrawTextW(
        deviceContext,
        text,
        -1,
        &bounds,
        DT_CENTER |
            DT_VCENTER |
            DT_SINGLELINE |
            DT_END_ELLIPSIS
    );

    SelectObject(
        deviceContext,
        oldFont
    );

    DeleteObject(font);
}

void ShellRenderer::DrawStartButton(
    HDC deviceContext,
    const RECT& bounds,
    bool startPanelOpen
) const
{
    HFONT font =
        CreateFontW(
            -26,
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY,
            DEFAULT_PITCH,
            L"Segoe MDL2 Assets"
        );

    if (!font)
    {
        return;
    }

    HGDIOBJ oldFont =
        SelectObject(
            deviceContext,
            font
        );

    const int oldBackgroundMode =
        SetBkMode(
            deviceContext,
            TRANSPARENT
        );

    const COLORREF oldTextColor =
        SetTextColor(
            deviceContext,
            startPanelOpen
                ? AccentColor
                : TextColor
        );

    RECT iconBounds = bounds;

    DrawTextW(
        deviceContext,
        L"\xE700",
        1,
        &iconBounds,
        DT_CENTER |
            DT_VCENTER |
            DT_SINGLELINE |
            DT_NOPREFIX
    );

    SetTextColor(
        deviceContext,
        oldTextColor
    );

    SetBkMode(
        deviceContext,
        oldBackgroundMode
    );

    SelectObject(
        deviceContext,
        oldFont
    );

    DeleteObject(font);
}

void ShellRenderer::DrawApplicationCapsule(
    HWND window,
    HDC deviceContext,
    const WindowTracker& windowTracker,
    bool startPanelOpen
) const
{
    PaintSurface(
        window,
        deviceContext,
        44
    );

    RECT startButtonBounds{
        18,
        8,
        74,
        64
    };

    DrawStartButton(
        deviceContext,
        startButtonBounds,
        startPanelOpen
    );

    const auto& applications =
        windowTracker.GetWindows();

    constexpr int buttonWidth = 56;
    constexpr int buttonHeight = 56;
    constexpr int buttonGap = 8;
    constexpr int iconSize = 32;
    constexpr int firstApplicationX = 82;
    constexpr int rightLimit = 682;

    int x = firstApplicationX;

    for (
        const auto& application :
        applications
    )
    {
        if (
            x + buttonWidth >
            rightLimit
        )
        {
            break;
        }

        RECT buttonBounds{
            x,
            8,
            x + buttonWidth,
            64
        };

        if (
            GetForegroundWindow() ==
            application.hwnd
        )
        {
            HBRUSH activeBrush =
                CreateSolidBrush(
                    RGB(48, 48, 48)
                );

            HPEN activePen =
                CreatePen(
                    PS_SOLID,
                    1,
                    AccentColor
                );

            if (
                activeBrush &&
                activePen
            )
            {
                HGDIOBJ oldBrush =
                    SelectObject(
                        deviceContext,
                        activeBrush
                    );

                HGDIOBJ oldPen =
                    SelectObject(
                        deviceContext,
                        activePen
                    );

                RoundRect(
                    deviceContext,
                    buttonBounds.left,
                    buttonBounds.top,
                    buttonBounds.right,
                    buttonBounds.bottom,
                    16,
                    16
                );

                SelectObject(
                    deviceContext,
                    oldPen
                );

                SelectObject(
                    deviceContext,
                    oldBrush
                );
            }

            if (activePen)
            {
                DeleteObject(activePen);
            }

            if (activeBrush)
            {
                DeleteObject(activeBrush);
            }
        }

        if (application.icon)
        {
            const int iconX =
                x +
                (
                    buttonWidth -
                    iconSize
                ) / 2;

            const int iconY =
                8 +
                (
                    buttonHeight -
                    iconSize
                ) / 2;

            DrawIconEx(
                deviceContext,
                iconX,
                iconY,
                application.icon,
                iconSize,
                iconSize,
                0,
                nullptr,
                DI_NORMAL
            );
        }
        else
        {
            DrawCenteredText(
                deviceContext,
                buttonBounds,
                L"□",
                22
            );
        }

        HBRUSH indicatorBrush =
            CreateSolidBrush(
                AccentColor
            );

        if (indicatorBrush)
        {
            RECT indicatorBounds{
                x + 24,
                59,
                x + 32,
                62
            };

            FillRect(
                deviceContext,
                &indicatorBounds,
                indicatorBrush
            );

            DeleteObject(
                indicatorBrush
            );
        }

        x +=
            buttonWidth +
            buttonGap;
    }
}

void ShellRenderer::DrawSystemCapsule(
    HWND window,
    HDC deviceContext
) const
{
    PaintSurface(
        window,
        deviceContext,
        40
    );

    SYSTEMTIME localTime{};
    GetLocalTime(&localTime);

    wchar_t text[96]{};

    swprintf_s(
        text,
        L"⌃    Wi-Fi    🔊    "
        L"%02u:%02u    "
        L"%02u/%02u/%04u",
        localTime.wHour,
        localTime.wMinute,
        localTime.wDay,
        localTime.wMonth,
        localTime.wYear
    );

    RECT clientRect{};

    GetClientRect(
        window,
        &clientRect
    );

    DrawCenteredText(
        deviceContext,
        clientRect,
        text,
        16
    );
}

void ShellRenderer::DrawStartPanel(
    HWND window,
    HDC deviceContext
) const
{
    PaintSurface(
        window,
        deviceContext,
        40
    );

    RECT titleBounds{
        28,
        20,
        490,
        58
    };

    HFONT titleFont =
        CreateTextFont(
            24,
            FW_BOLD
        );

    if (titleFont)
    {
        HGDIOBJ oldFont =
            SelectObject(
                deviceContext,
                titleFont
            );

        DrawTextW(
            deviceContext,
            L"CustomShellBar",
            -1,
            &titleBounds,
            DT_LEFT |
                DT_VCENTER |
                DT_SINGLELINE
        );

        SelectObject(
            deviceContext,
            oldFont
        );

        DeleteObject(titleFont);
    }

    RECT searchBounds{
        28,
        77,
        492,
        123
    };

    HBRUSH searchBrush =
        CreateSolidBrush(
            RGB(38, 38, 38)
        );

    HPEN searchPen =
        CreatePen(
            PS_SOLID,
            1,
            RGB(90, 90, 90)
        );

    if (
        searchBrush &&
        searchPen
    )
    {
        HGDIOBJ oldBrush =
            SelectObject(
                deviceContext,
                searchBrush
            );

        HGDIOBJ oldPen =
            SelectObject(
                deviceContext,
                searchPen
            );

        RoundRect(
            deviceContext,
            searchBounds.left,
            searchBounds.top,
            searchBounds.right,
            searchBounds.bottom,
            20,
            20
        );

        SelectObject(
            deviceContext,
            oldPen
        );

        SelectObject(
            deviceContext,
            oldBrush
        );
    }

    if (searchPen)
    {
        DeleteObject(searchPen);
    }

    if (searchBrush)
    {
        DeleteObject(searchBrush);
    }

    const COLORREF previousColor =
        SetTextColor(
            deviceContext,
            RGB(200, 200, 200)
        );

    RECT searchTextBounds{
        50,
        77,
        480,
        123
    };

    HFONT searchFont =
        CreateTextFont(15);

    if (searchFont)
    {
        HGDIOBJ oldFont =
            SelectObject(
                deviceContext,
                searchFont
            );

        DrawTextW(
            deviceContext,
            L"Search apps and files",
            -1,
            &searchTextBounds,
            DT_LEFT |
                DT_VCENTER |
                DT_SINGLELINE
        );

        SelectObject(
            deviceContext,
            oldFont
        );

        DeleteObject(searchFont);
    }

    SetTextColor(
        deviceContext,
        TextColor
    );

    RECT pinnedTitleBounds{
        28,
        145,
        490,
        180
    };

    DrawCenteredText(
        deviceContext,
        pinnedTitleBounds,
        L"Pinned apps prototype",
        17
    );

    constexpr const wchar_t* appNames[]
    {
        L"Explorer",
        L"Edge",
        L"Settings",
        L"Terminal",
        L"Code",
        L"Music",
        L"Photos",
        L"Calculator"
    };

    int index = 0;

    for (
        const wchar_t* appName :
        appNames
    )
    {
        const int column =
            index % 4;

        const int row =
            index / 4;

        RECT tileBounds{
            28 + column * 116,
            195 + row * 105,
            132 + column * 116,
            285 + row * 105
        };

        HBRUSH tileBrush =
            CreateSolidBrush(
                RGB(31, 31, 31)
            );

        if (tileBrush)
        {
            FillRect(
                deviceContext,
                &tileBounds,
                tileBrush
            );

            DeleteObject(
                tileBrush
            );
        }

        DrawCenteredText(
            deviceContext,
            tileBounds,
            appName,
            13
        );

        ++index;
    }

    RECT quickBounds{
        28,
        420,
        490,
        460
    };

    DrawCenteredText(
        deviceContext,
        quickBounds,
        L"Quick controls    "
        L"Volume 70%    "
        L"Wi-Fi    "
        L"Bluetooth",
        14
    );

    SetTextColor(
        deviceContext,
        RGB(255, 190, 150)
    );

    RECT exitBounds{
        28,
        555,
        492,
        608
    };

    DrawCenteredText(
        deviceContext,
        exitBounds,
        L"Exit and restore Windows taskbar",
        15
    );

    SetTextColor(
        deviceContext,
        previousColor
    );
}
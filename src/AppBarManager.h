#pragma once

#include <windows.h>
#include <shellapi.h>

class AppBarManager
{
public:
    AppBarManager() = default;
    ~AppBarManager();

    bool Register(HWND hwnd);
    void Unregister();

    void SetBottomPosition(HWND hwnd, int height);

private:
    APPBARDATA m_abd{};
    bool m_registered = false;
};
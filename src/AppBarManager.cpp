#include "AppBarManager.h"

AppBarManager::~AppBarManager()
{
    Unregister();
}

bool AppBarManager::Register(HWND hwnd)
{
    if (m_registered)
        return true;

    ZeroMemory(&m_abd, sizeof(m_abd));

    m_abd.cbSize = sizeof(APPBARDATA);
    m_abd.hWnd = hwnd;

    if (!SHAppBarMessage(ABM_NEW, &m_abd))
        return false;

    m_registered = true;

    return true;
}

void AppBarManager::Unregister()
{
    if (!m_registered)
        return;

    SHAppBarMessage(ABM_REMOVE, &m_abd);

    m_registered = false;
}

void AppBarManager::SetBottomPosition(
    HWND hwnd,
    int height
)
{
    if (!m_registered)
        return;

    RECT workArea{};
    SystemParametersInfo(
        SPI_GETWORKAREA,
        0,
        &workArea,
        0
    );

    m_abd.rc.left = workArea.left;
    m_abd.rc.right = workArea.right;
    m_abd.rc.bottom = workArea.bottom;
    m_abd.rc.top = workArea.bottom - height;

    m_abd.uEdge = ABE_BOTTOM;

    SHAppBarMessage(
        ABM_QUERYPOS,
        &m_abd
    );

    SHAppBarMessage(
        ABM_SETPOS,
        &m_abd
    );
}
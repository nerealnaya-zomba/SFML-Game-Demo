#include "AppIcon.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

void applyEmbeddedWindowIcon(sf::WindowBase& window)
{
#ifdef _WIN32
    HWND hwnd = reinterpret_cast<HWND>(window.getNativeHandle());
    if (!hwnd)
    {
        return;
    }

    static HICON largeIcon = static_cast<HICON>(LoadImageW(
        GetModuleHandleW(nullptr),
        L"MAINICON",
        IMAGE_ICON,
        GetSystemMetrics(SM_CXICON),
        GetSystemMetrics(SM_CYICON),
        LR_DEFAULTCOLOR
    ));
    static HICON smallIcon = static_cast<HICON>(LoadImageW(
        GetModuleHandleW(nullptr),
        L"MAINICON",
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR
    ));

    if (largeIcon)
    {
        SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(largeIcon));
    }
    if (smallIcon)
    {
        SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(smallIcon));
    }
#else
    (void)window;
#endif
}

#include <stdexcept>
#include <zg/interfaces/IPlatformWindow.hpp>
#ifdef _WIN32
#include <zg/windows/WIN32Window.hpp>
#endif
#ifdef MACOS
#include <zg/windows/MacOSWindow.hpp>
#endif
#ifdef __linux__
#include <zg/windows/XCBWindow.hpp>
#endif
#ifdef USE_WAYLAND
#include <zg/windows/WaylandWindow.hpp>
#endif
using namespace zg;
IPlatformWindow *zg::createPlatformWindow()
{
    if (SELECTED_WINDOW_TYPE == WINDOW_TYPE_WIN32)
    {
#ifdef _WIN32
        return new WIN32Window();
#endif
    }
    else if (SELECTED_WINDOW_TYPE == WINDOW_TYPE_MACOS)
    {
#ifdef MACOS
        return new MacOSWindow();
#endif
    }
    else if (SELECTED_WINDOW_TYPE == WINDOW_TYPE_IOS)
    {
#if defined(IOS)
        return new iOSWindow();
#endif
    }
#ifdef __linux__
    else if (SELECTED_WINDOW_TYPE == WINDOW_TYPE_XCB || SELECTED_WINDOW_TYPE == WINDOW_TYPE_X11 || SELECTED_WINDOW_TYPE == WINDOW_TYPE_WAYLAND)
    {

        auto xdgSessionType = getenv("XDG_SESSION_TYPE");
        if (strcmp(xdgSessionType, "x11") == 0)
        {
            return new XCBWindow();
        }
        else if (strcmp(xdgSessionType, "wayland") == 0)
        {
            return new WaylandWindow();
        }
    }
#endif
    else if (SELECTED_WINDOW_TYPE == WINDOW_TYPE_ANDROID)
    {
#ifdef ANDROID
        return new AndroidWindow();
#endif
    }
    throw std::runtime_error("Window type not supported!");
}
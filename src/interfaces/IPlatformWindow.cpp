#include <stdexcept>
#include <zg/interfaces/IPlatformWindow.hpp>
#ifdef USE_WIN32
#include <zg/windows/WIN32Window.hpp>
#endif
#ifdef USE_MACOS
#include <zg/windows/MacOSWindow.hpp>
#endif
#if defined(USE_XCB) || defined(USE_SWIFTSHADER)
#include <zg/windows/XCBWindow.hpp>
#endif
#ifdef USE_X11
#include <zg/windows/X11Window.hpp>
#endif
#ifdef USE_WAYLAND
#include <zg/windows/WaylandWindow.hpp>
#endif
using namespace zg;
IPlatformWindow* zg::createPlatformWindow()
{
    if (SELECTED_WINDOW_TYPE == WINDOW_TYPE_WIN32)
    {
#ifdef USE_WIN32
        return new WIN32Window();
#endif
    }
    else if (SELECTED_WINDOW_TYPE == WINDOW_TYPE_MACOS)
    {
#ifdef USE_MACOS
	    return new MacOSWindow();
#endif
    }
    else if (SELECTED_WINDOW_TYPE == WINDOW_TYPE_IOS)
    {
#ifdef USE_MACOS
	    return new iOSWindow();
#endif
    }
#ifdef LINUX
    else if (SELECTED_WINDOW_TYPE == WINDOW_TYPE_XCB || SELECTED_WINDOW_TYPE == WINDOW_TYPE_X11 || SELECTED_WINDOW_TYPE == WINDOW_TYPE_WAYLAND)
    {

		auto xdgSessionType = getenv("XDG_SESSION_TYPE");
		if (strcmp(xdgSessionType, "x11") == 0)
		{
#if defined(USE_XCB) || defined(USE_SWIFTSHADER)
            return new XCBWindow();
#endif
#ifdef USE_X11
	        return new X11Window();
#endif
		}
		else if (strcmp(xdgSessionType, "wayland") == 0)
		{
#ifdef USE_WAYLAND
	        return new WaylandWindow();
#endif
		}
    }
#endif
    else if (SELECTED_WINDOW_TYPE == WINDOW_TYPE_ANDROID)
    {
#ifdef USE_ANDROID
	    return new AndroidWindow();
#endif
    }
    throw std::runtime_error("Window type not supported!");
}
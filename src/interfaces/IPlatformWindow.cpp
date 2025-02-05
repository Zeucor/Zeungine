#include <stdexcept>
#include <zg/interfaces/IPlatformWindow.hpp>
#ifdef USE_WIN32
#include <zg/windows/WIN32Window.hpp>
#endif
#ifdef USE_MACOS
#include <zg/windows/MacOSWindow.hpp>
#endif
#ifdef USE_XCB
#include <zg/windows/XCBWindow.hpp>
#endif
#ifdef USE_X11
#include <zg/windows/X11Window.hpp>
#endif
using namespace zg;
IPlatformWindow* zg::createPlatformWindow()
{
    if (SUPPORTED_WINDOW_TYPES & WINDOW_TYPE_WIN32)
    {
#ifdef USE_WIN32
        return new WIN32Window();
#endif
    }
    else if (SUPPORTED_WINDOW_TYPES & WINDOW_TYPE_MACOS)
    {
#ifdef USE_MACOS
	    return new MacOSWindow();
#endif
    }
    else if (SUPPORTED_WINDOW_TYPES & WINDOW_TYPE_XCB)
    {
#ifdef USE_XCB
	    return new XCBWindow();
#endif
    }
    else if (SUPPORTED_WINDOW_TYPES & WINDOW_TYPE_X11)
    {
#ifdef USE_X11
	    return new X11Window();
#endif
    }
    throw std::runtime_error("Window type not supported!");
}
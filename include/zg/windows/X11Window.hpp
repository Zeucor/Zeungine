#pragma once
#ifdef __linux__
#ifdef USE_X11
#include "../Window.hpp"
#include <X11/Xutil.h>
#include <xcb/xcb.h>
#include <GL/glx.h>
typedef struct _XDisplay Display;
namespace zg
{
	struct X11Window : IPlatformWindow
	{
		Display *display = 0;
		xcb_connection_t *connection = 0;
		int32_t defaultRootWindow = 0;
		int32_t screen = 0;
		unsigned long window = 0;
		unsigned long rootWindow = 0;
		unsigned long wmDeleteWindow = 0;
		unsigned long wmProtocols = 0;
#ifdef USE_GL
		XVisualInfo visualinfo;
		GLXFBConfig bestFbc;
		GLXContext glcontext;
#endif
		void init(Window &window) override;
		void renderInit();
		void postInit() override;
		bool pollMessages() override;
		void destroy() override;
		void close() override;
		void minimize() override;
		void maximize() override;
		void restore() override;
		void warpPointer(glm::vec2 coords) override;
		void showPointer() override;
		void hidePointer() override;
		void setXY() override;
		void setWidthHeight() override;
		void mouseCapture(bool capture) override;
		void enableKeyAutoRepeat() override;
		void disableKeyAutoRepeat() override;
	};
}
#endif
#endif
#pragma once
#ifdef LINUX
#include "../RenderWindow.hpp"
#include <X11/Xutil.h>
#include <GL/glx.h>
typedef struct _XDisplay Display;
namespace zg::modules::gl
{
	struct X11Window : IPlatformWindow
	{
		Display *display = 0;
		int32_t defaultRootWindow = 0;
		int32_t screen = 0;
		unsigned long window = 0;
		unsigned long rootWindow = 0;
		unsigned long wmDeleteWindow;
		XVisualInfo visualinfo;
		GLXFBConfig bestFbc;
		GLXContext glcontext;
		void init(RenderWindow& window) override;
		void createContext() override;
		void renderInit();
		void postInit() override;
		bool pollMessages() override;
		void swapBuffers() override;
		void destroy() override;
		void close() override;
		void minimize() override;
		void maximize() override;
		void restore() override;
		void warpPointer(glm::vec2 coords) override;
		void setXY() override;
		void setWidthHeight() override;
		void mouseCapture(bool capture) override;
	};
}
#endif
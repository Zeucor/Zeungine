#pragma once
#ifdef _WIN32
#include "../Window.hpp"
#include <windows.h>
namespace zg
{
	struct WIN32Window : IPlatformWindow
	{
		HINSTANCE hInstance;
		HWND hwnd;
		HDC hDeviceContext;
		HGLRC hRenderingContext;
		float dpiScale = 1.0f;
		void init(Window& window) override;
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
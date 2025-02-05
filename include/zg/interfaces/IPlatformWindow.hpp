#pragma once
#include <memory>
#include <zg/glm.hpp>
#include "./IRenderer.hpp"
namespace zg
{
	struct Window;
	struct IPlatformWindow
	{
		Window *renderWindowPointer = nullptr;
		virtual ~IPlatformWindow() = default;
		virtual void init(Window &renderWindow) = 0;
		virtual void postInit() = 0;
		virtual bool pollMessages() = 0;
		virtual void swapBuffers() = 0;
		virtual void destroy() = 0;
		virtual void close() = 0;
		virtual void minimize() = 0;
		virtual void maximize() = 0;
		virtual void restore() = 0;
		virtual void warpPointer(glm::vec2 coords) = 0;
		virtual void showPointer() = 0;
		virtual void hidePointer() = 0;
		virtual void setXY() = 0;
		virtual void setWidthHeight() = 0;
		virtual void mouseCapture(bool capture) = 0;
	};
	inline static uint8_t WINDOW_TYPE_WIN32 = 1;
	inline static uint8_t WINDOW_TYPE_MACOS = 2;
	inline static uint8_t WINDOW_TYPE_X11 = 4;
	inline static uint8_t WINDOW_TYPE_XCB = 8;
	inline static uint8_t SUPPORTED_WINDOW_TYPES = ([]
	{
		uint8_t supported = 0;
#ifdef USE_WIN32
		supported |= WINDOW_TYPE_WIN32;
#endif
#ifdef USE_MACOS
		supported |= WINDOW_TYPE_MACOS;
#endif
#ifdef USE_X11
		supported |= WINDOW_TYPE_X11;
#endif
#ifdef USE_XCB
		supported |= WINDOW_TYPE_XCB;
#endif
		return supported;
	})();
	IPlatformWindow* createPlatformWindow();
}
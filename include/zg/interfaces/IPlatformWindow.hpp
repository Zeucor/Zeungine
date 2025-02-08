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
		uint8_t windowType = 0;
		virtual ~IPlatformWindow() = default;
		virtual void init(Window &renderWindow) = 0;
		virtual void postInit() = 0;
		virtual bool pollMessages() = 0;
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
	inline static constexpr uint8_t WINDOW_TYPE_WIN32 = 1;
	inline static constexpr uint8_t WINDOW_TYPE_MACOS = 2;
	inline static constexpr uint8_t WINDOW_TYPE_X11 = 4;
	inline static constexpr uint8_t WINDOW_TYPE_XCB = 8;
	inline static constexpr uint8_t WINDOW_TYPE_WAYLAND = 16;
	inline static constexpr uint8_t WINDOW_TYPE_ANDROID = 32;
	inline static constexpr uint8_t WINDOW_TYPE_IOS = 64;
	inline static uint8_t SELECTED_WINDOW_TYPE = ([]
												  {
		uint8_t selected = 0;
#ifdef USE_WIN32
		selected = WINDOW_TYPE_WIN32;
#endif
#ifdef USE_MACOS
		selected = WINDOW_TYPE_MACOS;
#endif
#ifdef USE_IOS
		selected = WINDOW_TYPE_IOS;
#endif
#ifdef USE_ANDROID
		selected = WINDOW_TYPE_ANDROID;
#endif
#ifdef LINUX
		auto xdgSessionType = getenv("XDG_SESSION_TYPE");
		if (strcmp(xdgSessionType, "x11") == 0)
		{
#if defined(USE_XCB) || defined(USE_SWIFTSHADER)
			selected = WINDOW_TYPE_XCB;
#elif defined(USE_X11)
			selected = WINDOW_TYPE_X11;
#endif
		}
		else if (strcmp(xdgSessionType, "wayland") == 0)
		{
#ifdef USE_WAYLAND
			selected = WINDOW_TYPE_WAYLAND;
#else
			throw std::runtime_error("XDG_SESSION_TYPE is 'wayland', yet Zeungine not built with Wayland support!")
#endif
		}
#endif
		return selected; })();
	IPlatformWindow *createPlatformWindow();
}
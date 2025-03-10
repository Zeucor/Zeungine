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
		virtual void enableKeyAutoRepeat() = 0;
		virtual void disableKeyAutoRepeat() = 0;
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
#ifdef _WIN32
		selected = WINDOW_TYPE_WIN32;
#endif
#ifdef MACOS
		selected = WINDOW_TYPE_MACOS;
#endif
#if defined(IOS)
		selected = WINDOW_TYPE_IOS;
#endif
#ifdef ANDROID
		selected = WINDOW_TYPE_ANDROID;
#endif
#ifdef __linux__
		auto xdgSessionType = getenv("XDG_SESSION_TYPE");
		if (strcmp(xdgSessionType, "x11") == 0)
		{
			selected = WINDOW_TYPE_XCB;
		}
		else if (strcmp(xdgSessionType, "wayland") == 0)
		{
			selected = WINDOW_TYPE_WAYLAND;
		}
#endif
		return selected; })();
	IPlatformWindow *createPlatformWindow();
}
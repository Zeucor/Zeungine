#pragma once
#ifdef __linux__
#if defined(USE_WAYLAND)
#include <linux/input.h>
#include <poll.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-util.h>
#include <wayland/wayland-xdg-shell-client-protocol.h>
#include <wayland/xdg-decoration-unstable-v1-client-protocol.h>
#include <zg/Window.hpp>
namespace zg
{
	struct WaylandWindow : IPlatformWindow
	{
		wl_registry *registry = 0;
		wl_shm *shm = 0;
		wl_compositor *compositor = 0;
		wl_display *display = 0;
		wl_surface *surface = 0;
		wl_seat *seat = 0;
		wl_pointer *seatPointer = 0;
		uint32_t serial = 0;
		glm::vec2 pointerCoords = glm::vec2(0);
		wl_keyboard *seatKeyboard = 0;
		xdg_wm_base *xdgWm = 0;
		xdg_surface *xdgSurface = 0;
		xdg_toplevel *xdgToplevel = 0;
		zxdg_decoration_manager_v1 *decorationManager = 0;
		zxdg_toplevel_decoration_v1 *toplevelDecoration = 0;
		bool shouldClose = false;
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
} // namespace zg
#endif
#endif
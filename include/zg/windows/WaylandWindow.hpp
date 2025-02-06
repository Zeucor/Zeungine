#pragma once
#ifdef LINUX
#if defined(USE_WAYLAND)
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-util.h>
#include "../Window.hpp"
namespace zg
{
	struct WaylandWindow : IPlatformWindow
	{
		wl_registry* registry = 0;
		wl_compositor* compositor = 0;
		wl_display* display = 0;
		wl_surface* surface = 0;
		wl_seat* seat = 0;
		wl_pointer* seatPointer = 0;
		wl_shell* shell = 0;
		wl_shm* shm = 0;
		void init(Window& window) override;
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
	};
} // namespace zg
#endif
#endif

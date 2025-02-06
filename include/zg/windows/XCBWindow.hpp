#pragma once
#ifdef LINUX
#ifdef USE_XCB
#include "../Window.hpp"
#include <xcb/xcb.h>
namespace zg
{
	struct XCBWindow : IPlatformWindow
	{
		xcb_connection_t* connection = 0;
		const xcb_setup_t* setup = 0;
		xcb_screen_t* screen = 0;
		xcb_window_t window = 0;
		xcb_atom_t wm_delete_window;
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
		void showPointer() override;
		void hidePointer() override;
		void setXY() override;
		void setWidthHeight() override;
		void mouseCapture(bool capture) override;
	};
}
#endif
#endif

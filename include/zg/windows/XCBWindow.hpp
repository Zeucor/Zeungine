#pragma once
#ifdef __linux__
#include "../Window.hpp"
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
namespace zg
{
	struct XCBWindow : IPlatformWindow
	{
		xcb_connection_t *connection = 0;
		const xcb_setup_t *setup = 0;
		xcb_screen_t *screen = 0;
		xcb_window_t window = 0;
		xcb_window_t root = 0;
		xcb_atom_t wm_protocols;
		xcb_atom_t wm_delete_window;
		xcb_atom_t atom_net_wm_state;
		xcb_atom_t atom_net_wm_state_hidden;
		xcb_atom_t atom_net_wm_state_maximized_horz;
		xcb_atom_t atom_net_wm_state_maximized_vert;
		xcb_key_symbols_t *keysyms;
		Display *display = 0;
		int32_t screenNumber = 0;
		void init(Window &window) override;
		void initAtoms();
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
}
#endif
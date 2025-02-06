#ifdef LINUX
#ifdef USE_XCB
#include <iostream>
#include <zg/common.hpp>
#include <zg/windows/XCBWindow.hpp>
using namespace zg;
void XCBWindow::init(Window& renderWindow)
{
	renderWindowPointer = &renderWindow;
	windowType = WINDOW_TYPE_XCB;
	connection = xcb_connect(nullptr, nullptr);
	if (xcb_connection_has_error(connection))
	{
		throw std::runtime_error("Failed to connect to X server!");
	}
	setup = xcb_get_setup(connection);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
	screen = iter.data;
	window = xcb_generate_id(connection);
	uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t value_list[] = {screen->white_pixel, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS};
	xcb_create_window(connection,
										XCB_COPY_FROM_PARENT, // depth
										window,
										screen->root, // parent
										renderWindow.windowX, renderWindow.windowY, // X, Y position
										renderWindow.windowWidth, renderWindow.windowHeight, // Width, Height
										1, // Border width
										XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, value_list);
	auto titleLength = strlen(renderWindow.title);
	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
											8, // Format (8-bit string)
											titleLength, // Length of the title
											renderWindow.title);
	xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(connection, 1, 12, "WM_PROTOCOLS");
	xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
	xcb_intern_atom_reply_t* wm_protocols_reply = xcb_intern_atom_reply(connection, wm_protocols_cookie, nullptr);
	xcb_intern_atom_reply_t* wm_delete_reply = xcb_intern_atom_reply(connection, wm_delete_cookie, nullptr);
	if (wm_protocols_reply && wm_delete_reply)
	{
		wm_delete_window = wm_delete_reply->atom;
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, wm_protocols_reply->atom, XCB_ATOM_ATOM, 32, 1,
												&wm_delete_window);
	}
	free(wm_protocols_reply);
	free(wm_delete_reply);
	xcb_map_window(connection, window);
	xcb_flush(connection);
}
void XCBWindow::postInit() {}
bool XCBWindow::pollMessages()
{
	xcb_generic_event_t* event;
	while ((event = xcb_poll_for_event(connection)))
	{
		switch (event->response_type & ~0x80)
		{
		case XCB_EXPOSE:
			std::cout << "Window exposed (redraw needed)." << std::endl;
			break;

		case XCB_KEY_PRESS:
			std::cout << "Key pressed! Exiting event loop." << std::endl;
			break;

		case XCB_CLIENT_MESSAGE:
			{
				xcb_client_message_event_t* cm = (xcb_client_message_event_t*)event;
				if (cm->data.data32[0] == wm_delete_window)
				{
					std::cout << "Window close requested." << std::endl;
					free(event);
					return false;
				}
				break;
			}

		case XCB_DESTROY_NOTIFY:
			std::cout << "Window destroyed." << std::endl;
			free(event);
			return false;

		default:
			break;
		}
		free(event);
	}
	return true;
}
void XCBWindow::destroy()
{
	xcb_destroy_window(connection, window);
	xcb_disconnect(connection);
}
void XCBWindow::close() {}
void XCBWindow::minimize() {}
void XCBWindow::maximize() {}
void XCBWindow::restore() {}
void XCBWindow::warpPointer(glm::vec2 coords) {}
void XCBWindow::showPointer() {}
void XCBWindow::hidePointer() {}
void XCBWindow::setXY() {}
void XCBWindow::setWidthHeight() {}
void XCBWindow::mouseCapture(bool capture) {}
#endif
#endif

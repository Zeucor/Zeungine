#ifdef LINUX
#ifdef USE_XCB
#include <iostream>
#include <zg/common.hpp>
#include <zg/windows/XCBWindow.hpp>
using namespace zg;
void XCBWindow::init(Window& renderWindow)
{
	renderWindowPointer = &renderWindow;
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
	xcb_map_window(connection, window);
	xcb_flush(connection);
}
void XCBWindow::postInit() {}
bool XCBWindow::pollMessages() { return true; }
void XCBWindow::swapBuffers() {}
void XCBWindow::destroy() {}
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

#ifdef __linux__
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/keysymdef.h>
#include <iostream>
#include <xcb/xfixes.h>
#include <xkbcommon/xkbcommon.h>
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
	root = screen->root;
	window = xcb_generate_id(connection);
	uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t value_list[] = {screen->white_pixel,
													 XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
														 XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS |
														 XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_FOCUS_CHANGE};
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
	keysyms = xcb_key_symbols_alloc(connection);
	if (!keysyms)
	{
		throw std::runtime_error("Failed to allocate key symbols!");
	}
	display = XOpenDisplay(0);
	if (!display)
	{
		throw std::runtime_error("Failed to openXlib display!");
	}
	XSync(display, False);
	screenNumber = DefaultScreen(display);
	initAtoms();
	XkbGetAutoRepeatRate(display, 0, &originalDelay, &originalInterval);
}
void XCBWindow::initAtoms()
{
	const char* atomNames[] = {"WM_PROTOCOLS",
														 "WM_DELETE_WINDOW",
														 "_NET_WM_STATE",
														 "_NET_WM_STATE_HIDDEN",
														 "_NET_WM_STATE_MAXIMIZED_HORZ",
														 "_NET_WM_STATE_MAXIMIZED_VERT"};

	xcb_intern_atom_cookie_t cookies[6];
	xcb_intern_atom_reply_t* replies[6];

	for (int i = 0; i < 6; i++)
	{
		cookies[i] = xcb_intern_atom(connection, 0, strlen(atomNames[i]), atomNames[i]);
	}

	for (int i = 0; i < 6; i++)
	{
		replies[i] = xcb_intern_atom_reply(connection, cookies[i], nullptr);
	}

	wm_protocols = replies[0] ? replies[0]->atom : XCB_ATOM_NONE;
	wm_delete_window = replies[1] ? replies[1]->atom : XCB_ATOM_NONE;
	atom_net_wm_state = replies[2] ? replies[2]->atom : XCB_ATOM_NONE;
	atom_net_wm_state_hidden = replies[3] ? replies[3]->atom : XCB_ATOM_NONE;
	atom_net_wm_state_maximized_horz = replies[4] ? replies[4]->atom : XCB_ATOM_NONE;
	atom_net_wm_state_maximized_vert = replies[5] ? replies[5]->atom : XCB_ATOM_NONE;

	for (int i = 0; i < 6; i++)
	{
		free(replies[i]);
	}
}
void XCBWindow::postInit() {}
bool XCBWindow::pollMessages()
{
	auto& window = *renderWindowPointer;
	xcb_generic_event_t* event;
	while ((event = xcb_poll_for_event(connection)))
	{
		auto eventType = event->response_type & ~0x80;
		switch (eventType)
		{
		case XCB_EXPOSE:
			break;

		case XCB_KEY_PRESS:
		case XCB_KEY_RELEASE:
			{
				auto pressed = eventType == XCB_KEY_PRESS;
				xcb_key_press_event_t* keyEvent = (xcb_key_press_event_t*)event;
				bool shiftPressed = keyEvent->state & (XCB_MOD_MASK_SHIFT);
				xcb_keysym_t keysym = xcb_key_symbols_get_keysym(keysyms, keyEvent->detail, shiftPressed ? 1 : 0);
				uint32_t keycode = xkb_keysym_to_utf32(keysym);
				int32_t mod = 0;
				if (keycode == 0)
				{
					switch (keysym)
					{
					case XK_Up:
						keycode = KEYCODE_UP;
						break;
					case XK_Down:
						keycode = KEYCODE_DOWN;
						break;
					case XK_Left:
						keycode = KEYCODE_LEFT;
						break;
					case XK_Right:
						keycode = KEYCODE_RIGHT;
						break;
					case XK_Home:
						keycode = KEYCODE_HOME;
						break;
					case XK_End:
						keycode = KEYCODE_END;
						break;
					case XK_Page_Up:
						keycode = KEYCODE_PGUP;
						break;
					case XK_Page_Down:
						keycode = KEYCODE_PGDOWN;
						break;
					case XK_Insert:
						keycode = KEYCODE_INSERT;
						break;
					case XK_Num_Lock:
						keycode = KEYCODE_NUMLOCK;
						break;
					case XK_Caps_Lock:
						keycode = KEYCODE_CAPSLOCK;
						break;
					case XK_Pause:
						keycode = KEYCODE_PAUSE;
						break;
					case XK_Super_L:
						keycode = KEYCODE_SUPER;
						break;
					case XK_Super_R:
						keycode = KEYCODE_SUPER;
						break;
					default:
						keycode = 0;
						break;
					}
				}
				window.handleKey(keycode, mod, pressed);
				break;
			}
		case XCB_CLIENT_MESSAGE:
			{
				xcb_client_message_event_t* cm = (xcb_client_message_event_t*)event;
				if (cm->data.data32[0] == wm_delete_window)
				{
					free(event);
					return false;
				}
				break;
			}
		case XCB_MOTION_NOTIFY:
			{
				xcb_motion_notify_event_t* motion = (xcb_motion_notify_event_t*)event;
				window.handleMouseMove(motion->event_x, window.windowHeight - motion->event_y);
				break;
			}
		case XCB_BUTTON_PRESS:
			{
				xcb_button_press_event_t* buttonPress = (xcb_button_press_event_t*)event;
				window.handleMousePress(buttonPress->detail - 1, true);
				break;
			}
		case XCB_BUTTON_RELEASE:
			{
				xcb_button_release_event_t* buttonRelease = (xcb_button_release_event_t*)event;
				auto button = buttonRelease->detail - 1;
				if (button == 3 || button == 4)
					break;
				window.handleMousePress(button, false);
				break;
			}
		case XCB_FOCUS_IN:
			window.callFocusHandler(true);
			break;

		case XCB_FOCUS_OUT:
			window.callFocusHandler(false);
			break;
		case XCB_DESTROY_NOTIFY:
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
	XkbSetAutoRepeatRate(display, 0, originalDelay, originalInterval);
	XCloseDisplay(display);
	xcb_destroy_window(connection, window);
	xcb_disconnect(connection);
}
void XCBWindow::close()
{
	xcb_client_message_event_t event = {};
	event.response_type = XCB_CLIENT_MESSAGE;
	event.window = window;
	event.format = 32;
	event.type = wm_protocols;
	event.data.data32[0] = wm_delete_window;
	event.data.data32[1] = XCB_CURRENT_TIME;
	xcb_send_event(connection, false, window, XCB_EVENT_MASK_NO_EVENT, (const char*)&event);
	xcb_flush(connection);
}
void XCBWindow::minimize()
{
	XIconifyWindow(display, window, screenNumber);
	XFlush(display);
}
void XCBWindow::maximize()
{
	xcb_client_message_event_t event = {};
	event.response_type = XCB_CLIENT_MESSAGE;
	event.window = window;
	event.format = 32;
	event.type = atom_net_wm_state;
	event.data.data32[0] = 1; // _NET_WM_STATE_ADD
	event.data.data32[1] = atom_net_wm_state_maximized_horz;
	event.data.data32[2] = atom_net_wm_state_maximized_vert;
	event.data.data32[3] = 0;
	event.data.data32[4] = 0;
	xcb_send_event(connection, false, root, XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
								 (const char*)&event);
	xcb_flush(connection);
}
void XCBWindow::restore()
{
	Atom wmState = XInternAtom(display, "_NET_WM_STATE", False);
	Atom maxHorz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	Atom maxVert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	XEvent event = {0};
	event.xclient.type = ClientMessage;
	event.xclient.message_type = wmState;
	event.xclient.display = display;
	event.xclient.window = window;
	event.xclient.format = 32;
	event.xclient.data.l[0] = 0;
	event.xclient.data.l[1] = maxHorz;
	event.xclient.data.l[2] = maxVert;
	event.xclient.data.l[3] = 0;
	event.xclient.data.l[4] = 0;
	XSendEvent(display, root, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
	XFlush(display);
}
void XCBWindow::warpPointer(glm::vec2 coords)
{
	xcb_warp_pointer(connection, XCB_NONE, window, 0, 0, 0, 0, static_cast<int16_t>(coords.x),
									 static_cast<int16_t>(coords.y));
	xcb_flush(connection);
}
void XCBWindow::showPointer()
{
	XFixesShowCursor(display, root);
	XSync(display, True);
}
void XCBWindow::hidePointer()
{
	XFixesHideCursor(display, root);
	XSync(display, True);
}
void XCBWindow::setXY()
{
	uint32_t values[] = {uint32_t(renderWindowPointer->windowX), uint32_t(renderWindowPointer->windowY)};
	xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
	xcb_flush(connection);
}
void XCBWindow::setWidthHeight()
{
	uint32_t values[] = {uint32_t(renderWindowPointer->windowWidth), uint32_t(renderWindowPointer->windowHeight)};
	xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
	xcb_flush(connection);
}
void XCBWindow::mouseCapture(bool capture)
{
	if (capture)
	{
		int result = XGrabPointer(display, window, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
															GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
		if (result != GrabSuccess)
		{
			std::cout << "Failed to GrabPointer" << std::endl;
		}
	}
	else
	{
		XUngrabPointer(display, CurrentTime);
	}
}
void XCBWindow::enableKeyAutoRepeat()
{
	XAutoRepeatOn(display);
}
void XCBWindow::disableKeyAutoRepeat()
{
	XAutoRepeatOff(display);
}
#endif

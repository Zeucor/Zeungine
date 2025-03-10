#ifdef __linux__
#ifdef USE_X11
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>
#include <xkbcommon/xkbcommon.h>
#include <iostream>
#include <zg/common.hpp>
#ifdef USE_GL
#include <GL/glext.h>
#endif
#include <zg/entities/Plane.hpp>
#include <zg/windows/X11Window.hpp>
#ifdef USE_EGL
#include <zg/renderers/EGLRenderer.hpp>
#endif
#include <iostream>
using namespace zg;
void X11Window::init(Window &renderWindow)
{
	renderWindowPointer = &renderWindow;
	windowType = WINDOW_TYPE_X11;
	XInitThreads();
	display = XOpenDisplay(0);
	const char *xserver = getenv("DISPLAY");
	if (display == 0)
	{
		throw std::runtime_error("Unable to open XDisplay!");
	}
	connection = xcb_connect(nullptr, nullptr);
	if (xcb_connection_has_error(connection))
	{
		throw std::runtime_error("Failed to connect to X server!");
	}
	defaultRootWindow = DefaultRootWindow(display);
	screen = DefaultScreen(display);
// #ifdef USE_GL
// 	static int visual_attribs[] = {GLX_X_RENDERABLE,
// 								   True,
// 								   GLX_DRAWABLE_TYPE,
// 								   GLX_WINDOW_BIT,
// 								   GLX_RENDER_TYPE,
// 								   GLX_RGBA_BIT,
// 								   GLX_X_VISUAL_TYPE,
// 								   GLX_TRUE_COLOR,
// 								   GLX_RED_SIZE,
// 								   8,
// 								   GLX_GREEN_SIZE,
// 								   8,
// 								   GLX_BLUE_SIZE,
// 								   8,
// 								   GLX_ALPHA_SIZE,
// 								   8,
// 								   GLX_DEPTH_SIZE,
// 								   24,
// 								   GLX_DOUBLEBUFFER,
// 								   True,
// 								   None};
// 	int glx_major, glx_minor;
// 	if (!glXQueryVersion(display, &glx_major, &glx_minor) || ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1))
// 	{
// 		return;
// 	}
// 	int fbcount;
// 	GLXFBConfig *fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
// 	if (!fbc)
// 	{
// 		return;
// 	}
// 	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999, best_alpha_mask = -1;
// 	for (int index = 0; index < fbcount; ++index)
// 	{
// 		XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[index]);
// 		if (vi)
// 		{
// 			int samp_buf, samples, red, green, blue, alpha, depth, transparentType;
// 			glXGetFBConfigAttrib(display, fbc[index], GLX_SAMPLE_BUFFERS, &samp_buf);
// 			glXGetFBConfigAttrib(display, fbc[index], GLX_SAMPLES, &samples);
// 			XRenderPictFormat *pict_format = XRenderFindVisualFormat(display, vi->visual);
// 			if (!pict_format)
// 				continue;
// 			if (best_fbc < 0 ||
// 				(samp_buf && samples > best_num_samp) || (pict_format->direct.alphaMask > best_alpha_mask))
// 				best_fbc = index, best_num_samp = samples, best_alpha_mask = pict_format->direct.alphaMask;
// 			if (worst_fbc < 0 || !samp_buf || samples < worst_num_samp)
// 				worst_fbc = index, worst_num_samp = samples;
// 		}
// 		XFree(vi);
// 	}
// 	bestFbc = fbc[best_fbc];
// 	XFree(fbc);
// 	XVisualInfo *vi = glXGetVisualFromFBConfig(display, bestFbc);
// 	Colormap cmap;
// 	rootWindow = RootWindow(display, vi->screen);
// 	XSetWindowAttributes attr;
// 	attr.colormap = cmap = XCreateColormap(display, rootWindow, vi->visual, AllocNone);
// 	attr.background_pixmap = None;
// 	attr.border_pixel = 0;
// 	attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask |
// 					  ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | FocusChangeMask;
// #elif defined(USE_EGL) || defined(USE_VULKAN)
	rootWindow = DefaultRootWindow(display);
	XSetWindowAttributes attr;
	attr.colormap = XCreateColormap(display, rootWindow, DefaultVisual(display, 0), AllocNone);
	attr.background_pixmap = None;
	attr.border_pixel = 0;
	attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask |
					  ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | FocusChangeMask;
// #endif
	int screenWidth = DisplayWidth(display, screen);
	int screenHeight = DisplayHeight(display, screen);
	int32_t windowWidth = renderWindow.windowWidth, windowHeight = renderWindow.windowHeight;
	int32_t windowX = renderWindow.windowX == -1 ? ((screenWidth - windowWidth) / 2) : renderWindow.windowX,
			windowY = renderWindow.windowY == -1 ? (screenHeight - windowHeight) / 2 : renderWindow.windowY;
	renderWindow.windowX = windowX;
	renderWindow.windowY = windowY;
#ifdef USE_GL
	window =
		XCreateWindow(display, RootWindow(display, vi->screen), windowX, windowY, windowWidth, windowHeight, 0, vi->depth,
					  InputOutput, vi->visual, CWColormap | CWEventMask | CWBackPixmap | CWBorderPixel, &attr);
	XFree(vi);
#elif defined(USE_EGL) || defined(USE_VULKAN)
	window = XCreateWindow(display, rootWindow, windowX, windowY, windowWidth, windowHeight, 0, CopyFromParent,
						   InputOutput, DefaultVisual(display, screen), CWColormap | CWEventMask | CWBackPixmap | CWBorderPixel, &attr);
#endif
	if (!window)
	{
		return;
	}
	if (strlen(renderWindow.title))
	{
		XStoreName(display, window, renderWindow.title);
		XClassHint *classHint = XAllocClassHint();
		if (classHint)
		{
			classHint->res_name = classHint->res_class = (char *)renderWindow.title;
			XSetClassHint(display, window, classHint);
			XFree(classHint);
		}
	}
	XSizeHints sizehints;
	sizehints.flags = PSize;
	if (renderWindow.windowX != -1 || renderWindow.windowY != -1)
	{
		sizehints.flags |= PPosition;
	}
	else
	{
		Atom netWmFullPlacement = XInternAtom(display, "_NET_WM_FULL_PLACEMENT", True);
		if (netWmFullPlacement != None)
		{
			long value = 1;
			XChangeProperty(display, window, netWmFullPlacement, XA_CARDINAL, 32, PropModeReplace,
							(unsigned char *)&value, 1);
		}
	}
	sizehints.x = windowX;
	sizehints.y = windowY;
	sizehints.width = windowWidth;
	sizehints.height = windowHeight;
	XSetWMNormalHints(display, window, &sizehints);
	wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", 0);
	wmProtocols = XInternAtom(display, "WM_PROTOCOLS", True);
	XSetWMProtocols(display, window, &wmDeleteWindow, 1);
	int supported_rtrn = 0;
	XkbSetDetectableAutoRepeat(display, true, &supported_rtrn);
	XAutoRepeatOff(display);
	if (renderWindow.borderless)
	{
		struct
		{
			unsigned long flags;
			unsigned long functions;
			unsigned long decorations;
			long input_mode;
			unsigned long status;
		} motifHints = {2, 0, 0, 0, 0};
		Atom motifHintsAtom = XInternAtom(display, "_MOTIF_WM_HINTS", False);
		XChangeProperty(display, window, motifHintsAtom, motifHintsAtom, 32, PropModeReplace,
						(unsigned char *)&motifHints, sizeof(motifHints) / sizeof(long));
	}
}
void X11Window::postInit()
{
	XMapRaised(display, window);
	// if (instance.fullscreen)
	// {
	// 	Atom atoms[2] = {XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False), None};
	// 	XChangeProperty(display, window, XInternAtom(display, "_NET_WM_STATE", False),
	// 					XA_ATOM, 32, PropModeReplace, (uInteger8 *)atoms, 1);
	// }
	XSync(display, False);
	XFlush(display);
}
bool X11Window::pollMessages()
{
	glm::vec2 pointerXY(-1);
	std::vector<std::tuple<uint32_t, uint32_t, bool>> buttons;
	while (display && XPending(display) > 0)
	{
		XEvent event;
		XNextEvent(display, &event);
		switch (event.type)
		{
		case ClientMessage:
		{
			if (event.xclient.message_type == XInternAtom(display, "WM_PROTOCOLS", 1) &&
				event.xclient.data.l[0] == XInternAtom(display, "WM_DELETE_WINDOW", 1))
			{
				return false;
			}
			break;
		};
		case ButtonPress:
		{
			auto button = event.xbutton.button - 1;
			renderWindowPointer->handleMousePress(button, true);
			break;
		};
		case ButtonRelease:
		{
			auto button = event.xbutton.button - 1;
			if (button == 3 || button == 4)
			{
				break;
			}
			renderWindowPointer->handleMousePress(button, false);
			break;
		};
		case MotionNotify:
		{
			pointerXY = {event.xmotion.x, event.xmotion.y};
			break;
		};
		case KeyPress:
		case KeyRelease:
		{
			auto keysym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, event.xkey.state & ShiftMask ? 1 : 0);
			auto mod = ((event.xkey.state & ControlMask) ? 1 : 0) |
					   ((event.xkey.state & ShiftMask) ? 2 : 0) |
					   ((keysym == XK_Super_L || keysym == XK_Super_R) ? 8 : 0); /* |
							  ((GetKeyState(VK_MENU) & 0x8000) >> 13) |
							  (((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) >> 12)*/
			;
			auto keycode = xkb_keysym_to_utf32(keysym);
			if (!keycode)
			{
				static std::unordered_map<uint32_t, uint32_t> specialKeyMap = {
					{XK_Home, KEYCODE_HOME},
					{XK_KP_Home, KEYCODE_HOME},
					{XK_End, KEYCODE_END},
					{XK_KP_End, KEYCODE_END},
					{XK_Page_Up, KEYCODE_PGUP},
					{XK_KP_Page_Up, KEYCODE_PGUP},
					{XK_Page_Down, KEYCODE_PGDOWN},
					{XK_KP_Page_Down, KEYCODE_PGDOWN},
					{XK_Insert, KEYCODE_INSERT},
					{XK_KP_Insert, KEYCODE_INSERT},
					{XK_Left, KEYCODE_LEFT},
					{XK_KP_Left, KEYCODE_LEFT},
					{XK_Right, KEYCODE_RIGHT},
					{XK_KP_Right, KEYCODE_RIGHT},
					{XK_Up, KEYCODE_UP},
					{XK_KP_Up, KEYCODE_UP},
					{XK_Down, KEYCODE_DOWN},
					{XK_KP_Down, KEYCODE_DOWN},
					{XK_Num_Lock, KEYCODE_NUMLOCK},
					{XK_Caps_Lock, KEYCODE_CAPSLOCK},
					{XK_Pause, KEYCODE_PAUSE}};
				static auto specialKeyMapEnd = specialKeyMap.end();
				auto keycodeIter = specialKeyMap.find(keysym);
				if (keycodeIter == specialKeyMapEnd)
					std::cout << "[UnknownKP]:" << "Mod: " << mod << ", Keycode: " << keycode << ", Keysym: " << keysym << std::endl;
				else
					keycode = keycodeIter->second;
			}
			auto keypress = event.type == KeyPress;
			renderWindowPointer->handleKey(keycode, mod, keypress);
			break;
		};
		case FocusIn:
		{
			renderWindowPointer->callFocusHandler(true);
			break;
		};
		case FocusOut:
		{
			renderWindowPointer->callFocusHandler(false);
			break;
		};
		case ConfigureNotify:
		{
			int32_t width = event.xconfigure.width, height = event.xconfigure.height;
			if (width != 0 && width != renderWindowPointer->windowWidth && height != 0 && height != renderWindowPointer->windowHeight)
				renderWindowPointer->resize({width, height});
		};
		case Expose:
		{
			break;
		};
		// case SelectionNotify:
		// {
		// 	Atom TARGETSAtom = XInternAtom(display, "TARGETS", 0);
		// 	if (event.xselection.property == None)
		// 	{
		// 		// "Conversion could not be performed."
		// 	}
		// 	else if (event.xselection.target == TARGETSAtom)
		// 	{
		// 		Clipboard::onPrintTargetsEvent();
		// 	}
		// 	else
		// 	{
		// 		Clipboard::onGetEvent();
		// 	}
		// 	break;
		// };
		case SelectionClear:
		{
			break;
		};
			// case SelectionRequest:
			// {
			// 	auto &selectionRequestEvent = Clipboard::selectionRequestEvent = &event.xselectionrequest;
			// 	Atom TARGETSAtom = XInternAtom(display, "TARGETS", 0);
			// 	if (selectionRequestEvent->target == TARGETSAtom)
			// 	{
			// 		Clipboard::onSetEventTARGETS(TARGETSAtom);
			// 	}
			// 	else if (selectionRequestEvent->target == Clipboard::utf8Atom)
			// 	{
			// 		Clipboard::onSetEvent();
			// 	}
			// 	else
			// 	{
			// 		Clipboard::onSetEventNo();
			// 	}
			// 	break;
			// };
		}
		continue;
	}
	if (pointerXY.x != -1)
	{
		auto x = pointerXY.x;
		auto y = renderWindowPointer->windowHeight - pointerXY.y;
		renderWindowPointer->handleMouseMove(x, y);
	}
	for (auto &buttonTuple : buttons)
	{
		auto &button = std::get<1>(buttonTuple);
		auto &pressed = std::get<2>(buttonTuple);
		bool hadChildFocus = false;
		for (auto &childWindowPointer : renderWindowPointer->childWindows)
		{
			auto &childWindow = *childWindowPointer;
			if (childWindow.minimized)
				continue;
			if (!childWindow.focused)
				continue;
			childWindow.windowButtons[button] = pressed;
			hadChildFocus = true;
			break;
		}
		if (!hadChildFocus)
			renderWindowPointer->windowButtons[button] = pressed;
	}
	return true;
}
void X11Window::destroy() { renderWindowPointer->iRenderer->destroy(); }
void X11Window::close()
{
	XEvent event;
	event.xclient.type = ClientMessage;
	event.xclient.window = window;
	event.xclient.message_type = wmProtocols;
	event.xclient.format = 32;
	event.xclient.data.l[0] = wmDeleteWindow;
	event.xclient.data.l[1] = CurrentTime;
	if (XSendEvent(display, window, False, NoEventMask, &event) == 0)
	{
		throw std::runtime_error("XSendEvent failed");
	}
	return;
}
void X11Window::minimize()
{
	XIconifyWindow(display, window, screen);
	XFlush(display);
}
void X11Window::maximize()
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
	event.xclient.data.l[0] = 1;
	event.xclient.data.l[1] = maxHorz;
	event.xclient.data.l[2] = maxVert;
	event.xclient.data.l[3] = 0;
	event.xclient.data.l[4] = 0;
	XSendEvent(display, rootWindow, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
	XFlush(display);
}
void X11Window::restore()
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
	XSendEvent(display, rootWindow, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
	XFlush(display);
}
void X11Window::warpPointer(glm::vec2 coords)
{
	XWarpPointer(display, None, window, 0, 0, 0, 0, coords.x, coords.y);
}
void X11Window::showPointer()
{
	XFixesShowCursor(display, rootWindow);
	XSync(display, True);
}
void X11Window::hidePointer()
{
	XFixesHideCursor(display, rootWindow);
	XSync(display, True);
}
void X11Window::setXY()
{
	XMoveWindow(display, window, renderWindowPointer->windowX, renderWindowPointer->windowY);
	XFlush(display);
}
void X11Window::setWidthHeight()
{
	XResizeWindow(display, window, renderWindowPointer->windowWidth, renderWindowPointer->windowHeight);
	XFlush(display);
}
void X11Window::mouseCapture(bool capture)
{
	if (capture)
	{
		int result = XGrabPointer(display, window, True,
								  ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
								  GrabModeAsync, GrabModeAsync,
								  None, None, CurrentTime);
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
void X11Window::enableKeyAutoRepeat()
{
	XAutoRepeatOn(display);
}
void X11Window::disableKeyAutoRepeat()
{
	XAutoRepeatOff(display);
}
#endif
#endif
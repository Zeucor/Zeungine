#ifdef _WIN32
#include <zg/windows/WIN32Window.hpp>
#include <zg/entities/Plane.hpp>
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")
#include <iostream>
using namespace zg;
static bool setDPIAware = false;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
static const uint32_t GL_KEYCODES[] = {0,27,49,50,51,52,53,54,55,56,57,48,45,61,8,9,81,87,69,82,84,89,85,73,79,80,91,93,10,0,65,83,68,70,71,72,74,75,76,59,39,96,0,92,90,88,67,86,66,78,77,44,46,47,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,KEYCODE_HOME,0,0,0,0,0,0,0,KEYCODE_END,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,17,3,0,20,0,19,0,5,18,4,26,127};
static LRESULT CALLBACK gl_wndproc(HWND hwnd, UINT msg, WPARAM wParam,
																	LPARAM lParam)
{
	struct Window* glWindow = (struct Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg)
	{
	case WM_CREATE:
		{
			CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
			glWindow = (Window*)createStruct->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)glWindow);
			break;
		};
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		{
			auto pressed = msg == WM_LBUTTONDOWN;
			bool hadChildFocus = false;
			for (auto &childWindowPointer : glWindow->childWindows)
			{
				auto &childWindow = *childWindowPointer;
				if (childWindow.minimized)
					continue;
				if (!childWindow.focused)
					continue;
				childWindow.windowButtons[0] = pressed;
				hadChildFocus = true;
				break;
			}
			if (!hadChildFocus)
				glWindow->windowButtons[0] = pressed;
			break;
		};
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		{
			auto pressed = msg == WM_RBUTTONDOWN;
			bool hadChildFocus = false;
			for (auto &childWindowPointer : glWindow->childWindows)
			{
				auto &childWindow = *childWindowPointer;
				if (childWindow.minimized)
					continue;
				if (!childWindow.focused)
					continue;
				childWindow.windowButtons[1] = pressed;
				hadChildFocus = true;
				break;
			}
			if (!hadChildFocus)
				glWindow->windowButtons[1] = pressed;
			break;
		};
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		{
			auto pressed = msg == WM_MBUTTONDOWN;
			bool hadChildFocus = false;
			for (auto &childWindowPointer : glWindow->childWindows)
			{
				auto &childWindow = *childWindowPointer;
				if (childWindow.minimized)
					continue;
				if (!childWindow.focused)
					continue;
				childWindow.windowButtons[2] = pressed;
				hadChildFocus = true;
				break;
			}
			if (!hadChildFocus)
				glWindow->windowButtons[2] = pressed;
			break;
		};
	case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam); // This gives the scroll amount
			auto wheelButton = zDelta > 0 ? 3 : 4; // Wheel scrolled up or down
			bool hadChildFocus = false;
			for (auto &childWindowPointer : glWindow->childWindows)
			{
				auto &childWindow = *childWindowPointer;
				if (childWindow.minimized)
					continue;
				if (!childWindow.focused)
					continue;
				childWindow.windowButtons[wheelButton] = 1;
				hadChildFocus = true;
				break;
			}
			if (!hadChildFocus)
				glWindow->windowButtons[wheelButton] = 1;
			break;
		};
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
		{
			WORD button = GET_XBUTTON_WPARAM(wParam);
			auto xButton = (button == XBUTTON2 ? 5 : (button == XBUTTON1 ? 6 : -1));
			if (xButton == -1)
				throw std::runtime_error("Invalid XButton");
			auto pressed = msg == WM_XBUTTONDOWN;
			bool hadChildFocus = false;
			for (auto &childWindowPointer : glWindow->childWindows)
			{
				auto &childWindow = *childWindowPointer;
				if (childWindow.minimized)
					continue;
				if (!childWindow.focused)
					continue;
				childWindow.windowButtons[xButton] = pressed;
				hadChildFocus = true;
				break;
			}
			if (!hadChildFocus)
				glWindow->windowButtons[xButton] = pressed;
			break;
		};
	case WM_MOUSEMOVE:
		{
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hwnd, &pt);
			auto x = pt.x;
			auto y = glWindow->windowHeight - pt.y;
			bool hadChildFocus = false;
			for (auto &childWindowPointer : glWindow->childWindows)
			{
				auto &childWindow = *childWindowPointer;
				if (childWindow.minimized)
					continue;
				if (!childWindow.focused)
					continue;
				auto childX = x - childWindow.windowX;
				auto childY = childWindow.windowHeight - (glWindow->windowHeight - y - childWindow.windowY);
				childWindow.mouseCoords.x = childX, childWindow.mouseCoords.y = childY;
				childWindow.mouseMoved = true;
				hadChildFocus = true;
				break;
			}
			if (!hadChildFocus)
			{
				glWindow->mouseCoords.y = y, glWindow->mouseCoords.x = x;
				glWindow->mouseMoved = true;
			}
			break;
		};
	case WM_KEYDOWN:
	case WM_KEYUP:
		{
			auto mod = ((GetKeyState(VK_CONTROL) & 0x8000) >> 15) |
							 ((GetKeyState(VK_SHIFT) & 0x8000) >> 14) |
							 ((GetKeyState(VK_MENU) & 0x8000) >> 13) |
							 (((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) >> 12);
			auto keycodeHiword = HIWORD(lParam) & 0x1ff;
			if (keycodeHiword < 0 || keycodeHiword > sizeof(GL_KEYCODES) / sizeof(GL_KEYCODES[0]))
			{
				break;
			}
			auto keycode = GL_KEYCODES[keycodeHiword];
			auto keypress = !((lParam >> 31) & 1);
			bool hadChildFocus = false;
			for (auto &childWindowPointer : glWindow->childWindows)
			{
				auto &childWindow = *childWindowPointer;
				if (childWindow.minimized)
					continue;
				if (!childWindow.focused)
					continue;
				childWindow.mod = mod;
				childWindow.windowKeys[keycode] = keypress;
				hadChildFocus = true;
				break;
			}
			if (!hadChildFocus)
			{
				glWindow->mod = mod;
				glWindow->windowKeys[keycode] = keypress;
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		{
			int32_t width = LOWORD(lParam), height = HIWORD(lParam);
			if (width != 0 && width != glWindow->windowWidth && height != 0 && height != glWindow->windowHeight)
				glWindow->resize({width, height});
			break;
		};
	case WM_SETFOCUS:
		{
			if (glWindow->focused)
				break;
			glWindow->focused = true;
			break;
		};
	case WM_KILLFOCUS:
		{
			if (!glWindow->focused)
				break;
			glWindow->focused = false;
			break;
		};
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
void SetupPixelFormat(HDC hDeviceContext)
{
	int pixelFormat;
	PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1 };
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pixelFormat = ChoosePixelFormat(hDeviceContext, &pfd);
	if (pixelFormat == 0)
	{
		throw std::runtime_error("pixelFormat is null!");
	}
	BOOL result = SetPixelFormat(hDeviceContext, pixelFormat, &pfd);
	if (!result)
	{
		throw std::runtime_error("failed to setPixelFormat!");
	}
}
void WIN32Window::init(Window &renderWindow)
{
	renderWindowPointer = &renderWindow;
	if (!setDPIAware)
	{
		HRESULT hr = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		if (FAILED(hr))
		{
			throw std::runtime_error("SetProcessDpiAwareness failed");
		}
		setDPIAware = true;
	}
	hInstance = GetModuleHandle(NULL);
	WNDCLASSEX wc = {0};
	// wc.cbSize = sizeof(WNDCLASS);
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = gl_wndproc;
	wc.hInstance = hInstance;
	wc.lpszClassName = renderWindow.title;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClassEx(&wc);
	dpiScale = 1.0f;
	HDC screen = GetDC(NULL);
	int32_t dpi = GetDeviceCaps(screen, LOGPIXELSX);
	ReleaseDC(NULL, screen);
	dpiScale = dpi / 96.0f;
	int adjustedWidth = renderWindow.windowWidth, adjustedHeight = renderWindow.windowHeight;
	auto wsStyle = WS_OVERLAPPEDWINDOW;
	RECT desiredRect = {0, 0, adjustedWidth, adjustedHeight};
	AdjustWindowRectEx(&desiredRect, wsStyle, FALSE, WS_EX_APPWINDOW);
	adjustedWidth = desiredRect.right - desiredRect.left;
	adjustedHeight = desiredRect.bottom - desiredRect.top;
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, renderWindow.title, renderWindow.title,
												wsStyle,
												renderWindow.windowX == -1 ? CW_USEDEFAULT : renderWindow.windowX,
												renderWindow.windowY == -1 ? CW_USEDEFAULT : renderWindow.windowY,
												adjustedWidth, adjustedHeight, 0, NULL, hInstance, renderWindowPointer);

	if (hwnd == NULL)
		throw std::runtime_error("Failed to create window");
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)renderWindowPointer);
	if (renderWindow.borderless)
	{
		SetWindowLong(hwnd, GWL_STYLE, (GetWindowLong(hwnd, GWL_STYLE) & ~WS_CAPTION & ~WS_THICKFRAME & ~WS_SYSMENU) | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
		UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
		if (renderWindow.windowX == -1 && renderWindow.windowY == -1)
			flags |= SWP_NOMOVE;
		SetWindowPos(hwnd,
			HWND_TOPMOST,
			(renderWindow.windowX == -1 ? 0 : renderWindow.windowX),          // Use explicit or default X position
			(renderWindow.windowY == -1 ? 0 : renderWindow.windowY),
			renderWindow.windowWidth,
			renderWindow.windowHeight, flags);
	}
	hDeviceContext = GetDC(hwnd);
	SetupPixelFormat(hDeviceContext);
}
void WIN32Window::postInit()
{
	ShowWindow(hwnd, SW_NORMAL);
	UpdateWindow(hwnd);
	RECT rect;
	if (GetWindowRect(hwnd, &rect))
	{
		renderWindowPointer->windowX = rect.left;
		renderWindowPointer->windowY = rect.top;
	}
}
bool WIN32Window::pollMessages()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}
void WIN32Window::swapBuffers()
{
	SwapBuffers(hDeviceContext);
}
void WIN32Window::destroy()
{
	renderWindowPointer->iRenderer->destroy();
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRenderingContext);
}
void WIN32Window::close()
{
	PostMessage(hwnd, WM_CLOSE, 0, 0);
}
void WIN32Window::minimize()
{
	ShowWindow(hwnd, SW_MINIMIZE);
}
void WIN32Window::maximize()
{
	ShowWindow(hwnd, SW_MAXIMIZE);
}
void WIN32Window::restore()
{
	ShowWindow(hwnd, SW_RESTORE);
}
void WIN32Window::warpPointer(glm::vec2 coords)
{
 	POINT pt;
 	pt.x = static_cast<LONG>(coords.x);
 	pt.y = static_cast<LONG>(coords.y);
 	ClientToScreen(hwnd, &pt);
 	auto currentWindow = renderWindowPointer;
 	while (currentWindow)
 	{
 		if (currentWindow->parentWindow)
 		{
 			pt.x += currentWindow->windowX;
 			pt.y += currentWindow->windowY;
 		}
 		currentWindow = currentWindow->parentWindow;
 	}
 	SetCursorPos(pt.x, pt.y);
}
void WIN32Window::setXY()
{
	auto &windowX = renderWindowPointer->windowX;
	auto &windowY = renderWindowPointer->windowY;
	auto &windowWidth = renderWindowPointer->windowWidth;
	auto &windowHeight = renderWindowPointer->windowHeight;
	UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
	if (windowX == -1 && windowY == -1)
		flags |= SWP_NOMOVE;
	SetWindowPos(hwnd,
		HWND_TOPMOST,
		(int)(windowX == -1 ? 0.f : windowX),
		(int)(windowY == -1 ? 0.f : windowY),
		(int)windowWidth,
		(int)windowHeight,
		flags);
}
void WIN32Window::setWidthHeight()
{
	auto &windowX = renderWindowPointer->windowX;
	auto &windowY = renderWindowPointer->windowY;
	auto &windowWidth = renderWindowPointer->windowWidth;
	auto &windowHeight = renderWindowPointer->windowHeight;
	float adjustedWidth = windowWidth, adjustedHeight = windowHeight;
	auto wsStyle = WS_OVERLAPPEDWINDOW;
	RECT desiredRect = {0, 0, (LONG)windowWidth, (LONG)windowHeight};
	AdjustWindowRectEx(&desiredRect, wsStyle, FALSE, WS_EX_APPWINDOW);
	adjustedWidth = desiredRect.right - desiredRect.left;
	adjustedHeight = desiredRect.bottom - desiredRect.top;
	UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
	if (windowX == -1 && windowY == -1)
		flags |= SWP_NOMOVE;
	SetWindowPos(hwnd,
		HWND_TOPMOST,
		(int)(windowX == -1 ? 0 : windowX),          // Use explicit or default X position
		(int)(windowY == -1 ? 0 : windowY),
		(int)adjustedWidth,
		(int)adjustedHeight, flags);
}
void WIN32Window::mouseCapture(bool capture)
{
	if (capture)
		SetCapture(hwnd);
	else
		ReleaseCapture();
}
std::shared_ptr<IPlatformWindow> zg::createPlatformWindow()
{
	return std::shared_ptr<IPlatformWindow>(new WIN32Window());
}
#endif
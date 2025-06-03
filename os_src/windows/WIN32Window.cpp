#ifdef _WIN32
#include <ShellScalingApi.h>
#include <zg/entities/Plane.hpp>
#include <zg/windows/WIN32Window.hpp>
#pragma comment(lib, "Shcore.lib")
#include <iostream>
using namespace zg;
static bool setDPIAware = false;
#ifdef USE_GL
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
#endif
static const uint32_t GL_KEYCODES[] = {
	0,	27, 49, 50, 51, 52, 53, 54,					 55, 56, 57, 48, 45, 61, 8,	 9,	 81, 87, 69, 82, 84, 89, 85, 73,
	79, 80, 91, 93, 10, KEYCODE_CTRL,	65, 83,					 68, 70, 71, 72, 74, 75, 76, 59, 39, 96, KEYCODE_SHIFT,	 92, 90, 88, 67, 86,
	66, 78, 77, 44, 46, 47, 0,	0,					 0,	 32, 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 KEYCODE_HOME,
	0,	0,	0,	0,	0,	0,	0,	KEYCODE_END, 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	0,	0,	0,	0,	0,	0,	0,					 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	0,	0,	0,	0,	0,	0,	0,					 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	0,	0,	0,	0,	0,	0,	0,					 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	0,	0,	0,	0,	0,	0,	0,					 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	0,	0,	0,	0,	0,	0,	0,					 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	0,	0,	0,	0,	0,	0,	0,					 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	0,	0,	0,	0,	0,	0,	0,					 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	0,	0,	0,	0,	0,	0,	0,					 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	0,	0,	0,	0,	0,	0,	0,					 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	0,	0,	0,	0,	0,	0,	0,	0,					 0,	 0,	 0,	 0,	 0,	 0,	 0,	 2,	 17, 3,	 0,	 20, 0,	 19, 0,	 5,
	18, 4,	26, 127};
static LRESULT CALLBACK gl_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		{
			auto pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
			auto button = 0;
			switch (msg)
			{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
				{
					button = 0;
					break;
				};
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
				{
					button = 1;
					break;
				};
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
				{
					button = 2;
					break;
				};
			}
			glWindow->queueEvent(EVENT_MOUSE_PRESS, pressed, button);
			break;
		};
	case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam); // This gives the scroll amount
			auto wheelButton = zDelta > 0 ? 3 : 4; // Wheel scrolled up or down
			glWindow->queueEvent(EVENT_MOUSE_PRESS, true, wheelButton);
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
			glWindow->queueEvent(EVENT_MOUSE_PRESS, pressed, xButton);
			break;
		};
	case WM_MOUSEMOVE:
		{
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hwnd, &pt);
			auto x = pt.x;
			auto y = *glWindow->windowHeight - pt.y;
			glWindow->queueEvent(EVENT_MOUSE_MOVE, glm::vec2(x, y));
			break;
		};
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		if (wParam == VK_MENU ||
			wParam == VK_LMENU ||
			wParam == VK_RMENU)
		{
			glWindow->queueEvent(EVENT_KEY_PRESS, msg == WM_SYSKEYDOWN, KEYCODE_ALT);
		}
		else
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		{
			auto mod = ((GetKeyState(VK_CONTROL) & 0x8000) >> 15) | ((GetKeyState(VK_SHIFT) & 0x8000) >> 14) |
				((GetKeyState(VK_MENU) & 0x8000) >> 13) | (((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) >> 12);
			auto keycodeHiword = HIWORD(lParam) & 0x1ff;
			if (keycodeHiword < 0 || keycodeHiword > sizeof(GL_KEYCODES) / sizeof(GL_KEYCODES[0]))
			{
				break;
			}
			auto keycode = GL_KEYCODES[keycodeHiword];
			auto keypress = !((lParam >> 31) & 1);
			BYTE keyboardState[256];
			GetKeyboardState(keyboardState);
			wchar_t translatedChar[2] = {};
			int result = ToUnicode(keycode, keycodeHiword, keyboardState, translatedChar, 2, 0);
			if (result > 0)
			{
				keycode = translatedChar[0];
			}
			glWindow->mod = mod;
			glWindow->queueEvent(EVENT_KEY_PRESS, keypress, keycode);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		{
			int32_t width = LOWORD(lParam), height = HIWORD(lParam);
			if (width != 0 && width != *glWindow->windowWidth && height != 0 && height != *glWindow->windowHeight)
				glWindow->resize({width, height});
			break;
		};
	case WM_SETFOCUS:
		{
			glWindow->queueFocusEvent(true);
			break;
		};
	case WM_KILLFOCUS:
		{
			glWindow->queueFocusEvent(false);
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
	PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR), 1};
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
void WIN32Window::init(Window& renderWindow)
{
	renderWindowPointer = &renderWindow;
	windowType = WINDOW_TYPE_WIN32;
	if (!setDPIAware)
	{
		HRESULT hr = SetProcessDPIAware();
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
	wc.lpszClassName = (*renderWindow.title).c_str();
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClassEx(&wc);
	dpiScale = 1.0f;
	HDC screen = GetDC(NULL);
	int32_t dpi = GetDeviceCaps(screen, LOGPIXELSX);
	ReleaseDC(NULL, screen);
	dpiScale = dpi / 96.0f;
	int adjustedWidth = (float&)renderWindow.windowWidth, adjustedHeight = (float&)renderWindow.windowHeight;
	auto wsStyle = WS_OVERLAPPEDWINDOW;
	RECT desiredRect = {0, 0, adjustedWidth, adjustedHeight};
	AdjustWindowRectEx(&desiredRect, wsStyle, FALSE, WS_EX_APPWINDOW);
	adjustedWidth = desiredRect.right - desiredRect.left;
	adjustedHeight = desiredRect.bottom - desiredRect.top;
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, (*renderWindow.title).c_str(), (*renderWindow.title).c_str(), wsStyle,
												(float&)renderWindow.windowX == -1 ? CW_USEDEFAULT : (float&)renderWindow.windowX,
												(float&)renderWindow.windowY == -1 ? CW_USEDEFAULT : (float&)renderWindow.windowY, adjustedWidth,
												adjustedHeight, 0, NULL, hInstance, renderWindowPointer);

	if (hwnd == NULL)
		throw std::runtime_error("Failed to create window");
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)renderWindowPointer);
	if (renderWindow.borderless)
	{
		SetWindowLong(hwnd, GWL_STYLE,
									(GetWindowLong(hwnd, GWL_STYLE) & ~WS_CAPTION & ~WS_THICKFRAME & ~WS_SYSMENU) | WS_MINIMIZEBOX |
										WS_MAXIMIZEBOX);
		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
		UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
		if ((float&)renderWindow.windowX == -1 && (float&)renderWindow.windowY == -1)
			flags |= SWP_NOMOVE;
		SetWindowPos(hwnd, HWND_TOPMOST,
								 ((float&)renderWindow.windowX == -1 ? 0 : (float&)renderWindow.windowX), // Use explicit or default X position
								 ((float&)renderWindow.windowY == -1 ? 0 : (float&)renderWindow.windowY), (float&)renderWindow.windowWidth,
								 (float&)renderWindow.windowHeight, flags);
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
void WIN32Window::destroy()
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRenderingContext);
}
void WIN32Window::close() { PostMessage(hwnd, WM_CLOSE, 0, 0); }
void WIN32Window::minimize() { ShowWindow(hwnd, SW_MINIMIZE); }
void WIN32Window::maximize() { ShowWindow(hwnd, SW_MAXIMIZE); }
void WIN32Window::restore() { ShowWindow(hwnd, SW_RESTORE); }
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
			pt.x += (const float&)currentWindow->windowX;
			pt.y += (const float&)currentWindow->windowY;
		}
		currentWindow = currentWindow->parentWindow;
	}
	SetCursorPos(pt.x, pt.y);
}
void WIN32Window::showPointer()
{
	// Force a WM_SETCURSOR message to be sent if the cursor is over the window
	// Get current cursor position
	POINT cursorPos;
	if (GetCursorPos(&cursorPos))
	{
		// Check if the cursor is over our window's client area
		RECT clientRect;
		if (GetClientRect(hwnd, &clientRect))
		{
			if (ScreenToClient(hwnd, &cursorPos))
			{
				if (PtInRect(&clientRect, cursorPos))
				{
					// If cursor is inside, SetCursor will be called via WM_SETCURSOR
					// We might need to invalidate or trigger a mouse move event
					// or simply rely on the next natural mouse move.
					// For immediate effect, you could call SetCursor directly here,
					// but handling WM_SETCURSOR is generally preferred.
					SetCursor(LoadCursor(NULL, IDC_ARROW)); // Show standard arrow
				}
				else
				{
					// Cursor is outside, ShowCursor might be needed if it was globally hidden
					while (::ShowCursor(TRUE) < 0)
						;
				}
			}
		}
	}
	// Fallback or if not over window, ensure global counter is positive
	while (::ShowCursor(TRUE) < 0)
		;
}
void WIN32Window::hidePointer()
{
	// Force a WM_SETCURSOR message if the cursor is over the window
	POINT cursorPos;
	if (GetCursorPos(&cursorPos))
	{
		RECT clientRect;
		if (GetClientRect(hwnd, &clientRect))
		{
			if (ScreenToClient(hwnd, &cursorPos))
			{
				if (PtInRect(&clientRect, cursorPos))
				{
					// If cursor is inside, SetCursor(NULL) will hide it via WM_SETCURSOR
					// Triggering WM_SETCURSOR might require moving the mouse slightly
					// or calling SetCursor directly for immediate effect.
					SetCursor(NULL); // Hide cursor immediately if over client area
				}
				else
				{
					// Cursor is outside, use ShowCursor(FALSE)
					while (::ShowCursor(FALSE) >= 0)
						;
				}
			}
		}
	}
	// Fallback or if not over window, ensure global counter is negative
	while (::ShowCursor(FALSE) >= 0)
		;
}
void WIN32Window::setXY()
{
	auto& windowX = *
	renderWindowPointer->windowX;
	auto& windowY = *renderWindowPointer->windowY;
	auto& windowWidth = *renderWindowPointer->windowWidth;
	auto& windowHeight = *renderWindowPointer->windowHeight;
	UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
	if (windowX == -1 && windowY == -1)
		flags |= SWP_NOMOVE;
	SetWindowPos(hwnd, HWND_TOPMOST, (int)(windowX == -1 ? 0.f : windowX), (int)(windowY == -1 ? 0.f : windowY),
							 (int)windowWidth, (int)windowHeight, flags);
}
void WIN32Window::setWidthHeight()
{
	auto& windowX = *renderWindowPointer->windowX;
	auto& windowY = *renderWindowPointer->windowY;
	auto& windowWidth = *renderWindowPointer->windowWidth;
	auto& windowHeight = *renderWindowPointer->windowHeight;
	float adjustedWidth = windowWidth, adjustedHeight = windowHeight;
	auto wsStyle = WS_OVERLAPPEDWINDOW;
	RECT desiredRect = {0, 0, (LONG)windowWidth, (LONG)windowHeight};
	AdjustWindowRectEx(&desiredRect, wsStyle, FALSE, WS_EX_APPWINDOW);
	adjustedWidth = desiredRect.right - desiredRect.left;
	adjustedHeight = desiredRect.bottom - desiredRect.top;
	UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
	if (windowX == -1 && windowY == -1)
		flags |= SWP_NOMOVE;
	SetWindowPos(hwnd, HWND_TOPMOST,
							 (int)(windowX == -1 ? 0 : windowX), // Use explicit or default X position
							 (int)(windowY == -1 ? 0 : windowY), (int)adjustedWidth, (int)adjustedHeight, flags);
}
void WIN32Window::mouseCapture(bool capture)
{
	if (capture)
		SetCapture(hwnd);
	else
		ReleaseCapture();
}
void WIN32Window::enableKeyAutoRepeat() {}
void WIN32Window::disableKeyAutoRepeat() {}
#endif
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFOEX miex;
	miex.cbSize = sizeof(miex);
	if (GetMonitorInfo(hMonitor, &miex))
	{
		DEVMODE dm;
		ZeroMemory(&dm, sizeof(dm));
		dm.dmSize = sizeof(dm);
		if (EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm))
		{
			auto& tuple = *reinterpret_cast<std::tuple<std::vector<ScreenMode>, std::vector<ScreenMode>>*>(dwData);
			auto& currentScreenModes = std::get<0>(tuple);
			auto& availableScreenModes = std::get<1>(tuple);
			ScreenMode* foundScreenModePointer = 0;
			glm::ivec2 size(dm.dmPelsWidth, dm.dmPelsHeight);
			RefreshRate_t refreshRate = static_cast<RefreshRate_t>(dm.dmDisplayFrequency);
			auto availableModeIter = std::find_if(
				availableScreenModes.begin(),
				availableScreenModes.end(), 
				[&](const auto& availableScreenMode) {
					return availableScreenMode.size == size &&
							availableScreenMode.refreshRate == refreshRate;
				}
			);
			if (availableModeIter != availableScreenModes.end())
			{
				currentScreenModes.push_back({ size, refreshRate, availableModeIter->index });
			}
		}
	}
	return TRUE; // Continue enumeration
};
std::vector<ScreenMode> WIN32Window::getCurrentScreenModes()
{
	auto availableScreenModes = getAvailableScreenModes();
	std::tuple<std::vector<ScreenMode>, std::vector<ScreenMode>> tuple({}, availableScreenModes);
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&tuple));
	return std::get<0>(tuple);
};
/*
 */
std::vector<ScreenMode> WIN32Window::getAvailableScreenModes()
{
	std::vector<ScreenMode> availableScreenModes;
	DEVMODE devMode;
	ZeroMemory(&devMode, sizeof(DEVMODE));
	devMode.dmSize = sizeof(DEVMODE);
	int modeIndex = 0;
	while (EnumDisplaySettings(NULL, modeIndex, &devMode))
	{
		glm::ivec2 screenSize(devMode.dmPelsWidth, devMode.dmPelsHeight);
		RefreshRate_t refreshRate = static_cast<RefreshRate_t>(devMode.dmDisplayFrequency);

		bool modeExists = false;
		for (auto& mode : availableScreenModes)
		{
			if (mode.size == screenSize &&
				mode.refreshRate == refreshRate)
			{
				modeExists = true;
				break;
			}
		}

		if (!modeExists)
		{
			availableScreenModes.push_back({ screenSize, refreshRate, static_cast<DWORD>(modeIndex) });
		}

		modeIndex++;
	}
	return availableScreenModes;
};
/*
 */
void WIN32Window::setScreenMode(const ScreenMode& screenMode)
{
	DEVMODE dm;
	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);
	dm.dmPelsWidth = screenMode.size.x;
	dm.dmPelsHeight = screenMode.size.y;
	dm.dmDisplayFrequency = static_cast<DWORD>(screenMode.refreshRate);
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lastMode);
	isModeChanged = true;
	LONG result = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
	currentMode = dm;
	if (result != DISP_CHANGE_SUCCESSFUL)
	{
		std::cerr << "Display change failed, error code: " << result << std::endl;
	}
};
/*
 */
void WIN32Window::restoreScreenMode()
{
	if (isModeChanged)
	{
		LONG result = ChangeDisplaySettings(&lastMode, 0);
		if (result != DISP_CHANGE_SUCCESSFUL)
		{
			std::cerr << "Failed to restore original display settings, error code: " << result << std::endl;
		}
		isModeChanged = false;
	}
};
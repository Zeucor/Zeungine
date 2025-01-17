#include <anex/modules/gl/GLWindow.hpp>
#include <stdexcept>
#include <iostream>
#include <anex/modules/gl/shaders/ShaderManager.hpp>
#include <anex/Logger.hpp>
#include <anex/modules/gl/shaders/ShaderFactory.hpp>
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")
#include <anex/modules/gl/textures/Texture.hpp>
#include <anex/modules/gl/entities/Plane.hpp>
using namespace anex::modules::gl;
#ifdef _WIN32
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
static bool setDPIAware = false;
GLWindow::GLWindow(const char* title, const int32_t& windowWidth, const int32_t& windowHeight, const int32_t& windowX, const int32_t& windowY, const bool &borderless, const uint32_t& framerate):
	IWindow(windowWidth, windowHeight, windowX, windowY, borderless, framerate),
	title(title),
	glContext(new GladGLContext),
	shaderContext(new ShaderContext)
{
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
	run();
};
GLWindow::GLWindow(GLWindow &parentWindow, GLScene &parentScene, const char *childTitle, const int32_t &childWindowWidth, const int32_t &childWindowHeight, const int32_t &childWindowX, const int32_t &childWindowY, const uint32_t &framerate):
	IWindow(childWindowWidth, childWindowHeight, childWindowX, childWindowY, false, framerate),
	title(childTitle),
	isChildWindow(true),
	parentWindow(&parentWindow),
	glContext(parentWindow.glContext),
	shaderContext(parentWindow.shaderContext),
	framebufferTexture(std::make_shared<textures::Texture>(parentWindow, glm::ivec4(childWindowWidth, childWindowHeight, 1, 0), (void*)0)),
	framebufferDepthTexture(std::make_shared<textures::Texture>(parentWindow, glm::ivec4(childWindowWidth, childWindowHeight, 1, 0), (void*)0)),
	framebuffer(std::make_shared<textures::Framebuffer>(parentWindow, *framebufferTexture, *framebufferDepthTexture)),
	framebufferPlane(std::make_shared<entities::Plane>(parentWindow, parentScene, glm::vec3(childWindowX + (childWindowWidth / 2), parentWindow.windowHeight - childWindowY - (childWindowHeight / 2), 0.5), glm::vec3(0), glm::vec3(1), glm::vec2(childWindowWidth, childWindowHeight), *framebufferTexture))
{
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
};

GLWindow::~GLWindow()
{
	if (!isChildWindow)
		delete glContext;
};
#ifdef _WIN32
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
typedef BOOL (APIENTRY * PFNWGLSWAPINTERVALEXTPROC) (int interval);
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;
static const uint8_t GL_KEYCODES[] = {0,27,49,50,51,52,53,54,55,56,57,48,45,61,8,9,81,87,69,82,84,89,85,73,79,80,91,93,10,0,65,83,68,70,71,72,74,75,76,59,39,96,0,92,90,88,67,86,66,78,77,44,46,47,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,17,3,0,20,0,19,0,5,18,4,26,127};
static LRESULT CALLBACK gl_wndproc(HWND hwnd, UINT msg, WPARAM wParam,
																	LPARAM lParam)
{
	struct GLWindow* glWindow = (struct GLWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg)
	{
	case WM_CREATE:
		{
			CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
			glWindow = (GLWindow*)createStruct->lpCreateParams;
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
			for (auto &childWindow : glWindow->childWindows)
			{
				if (!childWindow.focused)
				{
					continue;
				}
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
			for (auto &childWindow : glWindow->childWindows)
			{
				if (!childWindow.focused)
				{
					continue;
				}
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
			for (auto &childWindow : glWindow->childWindows)
			{
				if (!childWindow.focused)
				{
					continue;
				}
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
			for (auto &childWindow : glWindow->childWindows)
			{
				if (!childWindow.focused)
				{
					continue;
				}
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
			for (auto &childWindow : glWindow->childWindows)
			{
				if (!childWindow.focused)
				{
					continue;
				}
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
			for (auto &childWindow : glWindow->childWindows)
			{
				if (!childWindow.focused)
				{
					continue;
				}
				auto childX = x - childWindow.windowX;
				auto childY = glWindow->windowHeight - y - childWindow.windowY;
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
			auto keycode = GL_KEYCODES[HIWORD(lParam) & 0x1ff];
			auto keypress = !((lParam >> 31) & 1);
			bool hadChildFocus = false;
			for (auto &childWindow : glWindow->childWindows)
			{
				if (!childWindow.focused)
				{
					continue;
				}
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
#endif
void GLWindow::startWindow()
{
#ifdef _WIN32
	if (!setDPIAware)
	{
		HRESULT hr = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		if (FAILED(hr))
		{
			throw std::runtime_error("SetProcessDpiAwareness failed");
		}
		setDPIAware = true;
	}
	hInstance = isChildWindow ? parentWindow->hInstance : GetModuleHandle(NULL);
	WNDCLASSEX wc = {0};
	// wc.cbSize = sizeof(WNDCLASS);
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = gl_wndproc;
	wc.hInstance = hInstance;
	wc.lpszClassName = title;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClassEx(&wc);
	dpiScale = 1.0f;
	HDC screen = GetDC(NULL);
	int32_t dpi = GetDeviceCaps(screen, LOGPIXELSX);
	ReleaseDC(NULL, screen);
	dpiScale = dpi / 96.0f;
	int adjustedWidth = windowWidth, adjustedHeight = windowHeight;
	auto wsStyle = isChildWindow ? (WS_CHILD | WS_VISIBLE) : WS_OVERLAPPEDWINDOW;
	RECT desiredRect = {0, 0, (LONG)windowWidth, (LONG)windowHeight};
	AdjustWindowRectEx(&desiredRect, wsStyle, FALSE, WS_EX_APPWINDOW);
	adjustedWidth = desiredRect.right - desiredRect.left;
	adjustedHeight = desiredRect.bottom - desiredRect.top;
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, title, title,
												wsStyle,
												windowX == -1 ? CW_USEDEFAULT : windowX,
												windowY == -1 ? CW_USEDEFAULT : windowY,
												adjustedWidth, adjustedHeight, isChildWindow ? parentWindow->hwnd : 0, NULL, hInstance, this);

	if (hwnd == NULL)
		throw std::runtime_error("Failed to create window");
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
	if (borderless)
	{
		SetWindowLong(hwnd, GWL_STYLE, (GetWindowLong(hwnd, GWL_STYLE) & ~WS_CAPTION & ~WS_THICKFRAME & ~WS_SYSMENU) | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
		UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
		if (windowX == -1 && windowY == -1)
			flags |= SWP_NOMOVE;
		SetWindowPos(hwnd,
			HWND_TOPMOST,
			(windowX == -1 ? 0 : windowX),          // Use explicit or default X position
				(windowY == -1 ? 0 : windowY),
		windowWidth,
			windowHeight, flags);
	}
	hDeviceContext = GetDC(hwnd);
	renderInit();
	ShowWindow(hwnd, SW_NORMAL);
	UpdateWindow(hwnd);
	RECT rect;
	if (GetWindowRect(hwnd, &rect))
	{
		windowX = rect.left;
		windowY = rect.top;
	}
	MSG msg;
	while (true)
	{
		BOOL msgReceived = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (msgReceived)
		{
			if (msg.message == WM_QUIT)
				goto _exit;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		runRunnables();
		updateKeyboard();
		updateMouse();
		for (auto &childWindow : childWindows)
		{
			childWindow.render();
		}
		glContext->ClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		GLcheck(*this, "glClearColor");
		glContext->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		GLcheck(*this, "glClear");
		render();
		for (auto &childWindow : childWindows)
		{
			childWindow.framebufferPlane->render();
		}
		SwapBuffers(hDeviceContext);
	}
#endif
_exit:
	scene.reset();
	childWindows.clear();
	delete shaderContext;
#ifdef _WIN32
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRenderingContext);
#endif
};

#ifdef _WIN32
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
};
#endif

#ifdef _WIN32
void *get_proc(const char* name) {
	void* p = (void*)wglGetProcAddress(name);
	if(p == 0 ||
		 (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
		 (p == (void*)-1) )
	{
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void*)GetProcAddress(module, name);
	}
	return p;
}
#endif

void GLWindow::renderInit()
{
#ifdef _WIN32
	SetupPixelFormat(hDeviceContext);
	HGLRC hTempRC = wglCreateContext(hDeviceContext);
	wglMakeCurrent(hDeviceContext, hTempRC);
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	wglMakeCurrent(nullptr, nullptr);
	int attribList[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	hRenderingContext = wglCreateContextAttribsARB(hDeviceContext, 0, attribList);
	wglDeleteContext(hTempRC);
	wglMakeCurrent(hDeviceContext, hRenderingContext);
	gladLoadGLContext(glContext, (GLADloadfunc)get_proc);
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	glContext->Enable(GL_DEPTH_TEST);
	GLcheck(*this, "glEnable");
	glContext->Enable(GL_CULL_FACE);
	GLcheck(*this, "glEnable");
	glContext->CullFace(GL_BACK);
	GLcheck(*this, "glCullFace");
	glContext->FrontFace(GL_CCW);
	GLcheck(*this, "glFrontFace");
	glContext->Viewport(0, 0, windowWidth, windowHeight);
	GLcheck(*this, "glViewport");
	glContext->ClearDepth(1.0);
	GLcheck(*this, "glClearDepth");
	glContext->DepthRange(0.0, 1.0);
	GLcheck(*this, "glDepthRange");
	glContext->Enable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	GLcheck(*this, "glEnable:GL_SAMPLE_ALPHA_TO_COVERAGE");
	glContext->Enable(GL_BLEND);
	GLcheck(*this, "glEnable:GL_BLEND");
	glContext->Enable(GL_DITHER);
	GLcheck(*this, "glEnable:GL_DITHER");
	glContext->BlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
	GLcheck(*this, "glBlendEquationSeparate");
	glContext->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLcheck(*this, "glBlendFunc");
	glContext->Enable(GL_DEBUG_OUTPUT);
	GLcheck(*this, "glEnable");
	glContext->Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	GLcheck(*this, "glEnable");
	glContext->DebugMessageCallback([](GLuint source, GLuint type, GLuint id, GLuint severity, GLsizei length, const GLchar* message, const void* userParam) {
		if (type == GL_DEBUG_TYPE_OTHER)
		{
			return;
		}
		std::cerr << "OpenGL Debug Message: " << message << std::endl;
	}, nullptr);
	wglSwapIntervalEXT(1);
#endif
};

void GLWindow::updateKeyboard()
{
	for (unsigned int i = 0; i < 256; ++i)
	{
		auto& pressed = windowKeys[i];
		if (keys[i] != pressed)
		{
			callKeyPressHandler(i, pressed);
		}
		if (pressed)
		{
			callKeyUpdateHandler(i);
		}
	}
};

void GLWindow::updateMouse()
{
	for (unsigned int i = GLWindow::MinMouseButton; i < GLWindow::MaxMouseButton; ++i)
	{
_checkPressed:
		auto& pressed = windowButtons[i];
		if (buttons[i] != pressed)
		{
			callMousePressHandler(i, pressed);
			if ((i == 3 || i == 4) && pressed)
			{
				windowButtons[i] = false;
				goto _checkPressed;
			}
		}
	}
	if (mouseMoved)
	{
		callMouseMoveHandler(mouseCoords);
		mouseMoved = false;
	}
};

void GLWindow::close()
{
#ifdef _WIN32
  PostMessage(hwnd, WM_CLOSE, 0, 0);
#endif
};
void GLWindow::minimize()
{
#ifdef _WIN32
	ShowWindow(hwnd, SW_MINIMIZE);
	buttons.clear();
	for (unsigned i = 0; i <= GLWindow::MaxMouseButton; ++i)
	{
		windowButtons[i] = false;
	}
#endif
};
void GLWindow::maximize()
{
#ifdef _WIN32
	if (maximized)
	{
		maximized = false;
		ShowWindow(hwnd, SW_RESTORE);
	}
	else
	{
		maximized = true;
		ShowWindow(hwnd, SW_MAXIMIZE);
	}
#endif
};
void GLWindow::preRender()
{
	if (!isChildWindow)
	{
		return;
	}
  runRunnables();
	updateKeyboard();
	updateMouse();
	framebuffer->bind();
};
void GLWindow::postRender()
{
	if (!isChildWindow)
	{
		return;
	}
	framebuffer->unbind();
};
void GLWindow::drawLine(int x0, int y0, int x1, int y1, uint32_t color)
{
};
void GLWindow::drawRectangle(int x, int y, int w, int h, uint32_t color)
{
};
void GLWindow::drawCircle(int x, int y, int radius, uint32_t color)
{
};
void GLWindow::drawText(int x, int y, const char* text, int scale, uint32_t color)
{
};
const bool anex::modules::gl::GLcheck(GLWindow &window, const char* fn, const bool& egl)
{
	while (true)
	{
		uint32_t err = 0;
		if (!egl)
			err = window.glContext->GetError();
#if defined(_Android)
		else if (egl)
			err = eglGetError();
#endif
		if (err == GL_NO_ERROR
#if defined(_Android)
			|| err == EGL_SUCCESS
#endif
			)
		{
			/*|
			  |*/
			break;
		}
		switch (err)
		{
		case GL_INVALID_ENUM:
		{
			Logger::print(Logger::Error, "GL_INVALID_ENUM", "(", fn, "): ", "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.");
			break;
		};
		case GL_INVALID_VALUE:
		{
			Logger::print(Logger::Error, "GL_INVALID_VALUE", "(", fn, "): ", "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.");
			break;
		};
		case GL_INVALID_OPERATION:
		{
			Logger::print(Logger::Error, "GL_INVALID_OPERATION", "(", fn, "): ", "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.");
			break;
		};
		case GL_INVALID_FRAMEBUFFER_OPERATION:
		{
			Logger::print(Logger::Error, "GL_INVALID_FRAMEBUFFER_OPERATION", "(", fn, "): ", "The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.");
			break;
		};
		case GL_OUT_OF_MEMORY:
		{
			Logger::print(Logger::Error, "GL_OUT_OF_MEMORY", "(", fn, "): ", "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.");
			break;
		};
		case GL_STACK_UNDERFLOW:
		{
			Logger::print(Logger::Error, "GL_STACK_UNDERFLOW", "(", fn, "): ", "An attempt has been made to perform an operation that would cause an internal stack to underflow.");
			break;
		};
		case GL_STACK_OVERFLOW:
		{
			Logger::print(Logger::Error, "GL_STACK_OVERFLOW", "(", fn, "): ", "An attempt has been made to perform an operation that would cause an internal stack to overflow.");
			break;
		};
#if defined(_Android)
		case EGL_NOT_INITIALIZED:
		{
			Logger::print(Logger::Error, "EGL_NOT_INITIALIZED", "(", fn, "): ", "EGL is not initialized, or could not be initialized, for the specified EGL display connection.");
			break;
		};
		case EGL_BAD_ACCESS:
		{
			Logger::print(Logger::Error, "EGL_BAD_ACCESS", "(", fn, "): ", "EGL cannot access a requested resource (for example a context is bound in another thread).");
			break;
		};
		case EGL_BAD_ALLOC:
		{
			Logger::print(Logger::Error, "EGL_BAD_ALLOC", "(", fn, "): ", "EGL failed to allocate resources for the requested operation.");
			break;
		};
		case EGL_BAD_ATTRIBUTE:
		{
			Logger::print(Logger::Error, "EGL_BAD_ATTRIBUTE", "(", fn, "): ", "An unrecognized attribute or attribute value was passed in the attribute list.");
			break;
		};
		case EGL_BAD_CONTEXT:
		{
			Logger::print(Logger::Error, "EGL_BAD_CONTEXT", "(", fn, "): ", "An EGLContext argument does not name a valid EGL rendering context.");
			break;
		};
		case EGL_BAD_CONFIG:
		{
			Logger::print(Logger::Error, "EGL_BAD_CONFIG", "(", fn, "): ", "An EGLConfig argument does not name a valid EGL frame buffer configuration.");
			break;
		};
		case EGL_BAD_CURRENT_SURFACE:
		{
			Logger::print(Logger::Error, "EGL_BAD_CURRENT_SURFACE", "(", fn, "): ", "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid.");
			break;
		};
		case EGL_BAD_DISPLAY:
		{
			Logger::print(Logger::Error, "EGL_BAD_DISPLAY", "(", fn, "): ", "An EGLDisplay argument does not name a valid EGL display connection.");
			break;
		};
		case EGL_BAD_SURFACE:
		{
			Logger::print(Logger::Error, "EGL_BAD_SURFACE", "(", fn, "): ", "An EGLSurface argument does not name a valid surface (window, pixel buffer or pixmap) configured for GL rendering.");
			break;
		};
		case EGL_BAD_MATCH:
		{
			Logger::print(Logger::Error, "EGL_BAD_MATCH", "(", fn, "): ", "Arguments are inconsistent (for example, a valid context requires buffers not supplied by a valid surface).");
			break;
		};
		case EGL_BAD_PARAMETER:
		{
			Logger::print(Logger::Error, "EGL_BAD_PARAMETER", "(", fn, "): ", "One or more argument values are invalid.");
			break;
		};
		case EGL_BAD_NATIVE_PIXMAP:
		{
			Logger::print(Logger::Error, "EGL_BAD_NATIVE_PIXMAP", "(", fn, "): ", "A NativePixmapType argument does not refer to a valid native pixmap.");
			break;
		};
		case EGL_BAD_NATIVE_WINDOW:
		{
			Logger::print(Logger::Error, "EGL_BAD_NATIVE_WINDOW", "(", fn, "): ", "A NativeWindowType argument does not refer to a valid native window.");
			break;
		};
		case EGL_CONTEXT_LOST:
		{
			Logger::print(Logger::Error, "EGL_CONTEXT_LOST", "(", fn, "): ", "A power management event has occurred. The application must destroy all contexts and reinitialise OpenGL ES state and objects to continue rendering.");
			break;
		};
#endif
		}
	}
	return true;
};
void GLWindow::warpPointer(const glm::vec2 &coords)
{
#if defined(_UNIX)
	DEFINE_X11_WINDOW_DRIVER;
	XWarpPointer(x11WindowDriver.display, None, x11WindowDriver.window, 0, 0, 0, 0, position.x, position.y);
#elif defined(_WIN32)
	// Assuming 'hwnd' is the handle to the window
	POINT pt;
	pt.x = static_cast<LONG>(coords.x);
	pt.y = static_cast<LONG>(coords.y);
	// Convert the client (window-relative) coordinates to screen coordinates
	ClientToScreen(hwnd, &pt);
	// Set the cursor's position to the converted screen coordinates
	SetCursorPos(pt.x, pt.y);
	justWarpedPointer = true;
#endif
};
void GLWindow::setXY(const int32_t &x, const int32_t &y)
{
	windowX = x;
	windowY = y;
	UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
	if (windowX == -1 && windowY == -1)
		flags |= SWP_NOMOVE;
	SetWindowPos(hwnd,
		HWND_TOPMOST,
		(windowX == -1 ? 0 : windowX),          // Use explicit or default X position
		(windowY == -1 ? 0 : windowY),
		windowWidth,
		windowHeight,
		flags);
	// std::cout << "Setting Window X/Y to {" << windowX << ", " << windowY << "}" << std::endl;
};
void GLWindow::setWidthHeight(const uint32_t &width, const uint32_t &height)
{
	windowWidth = width;
	windowHeight = height;
	int adjustedWidth = windowWidth, adjustedHeight = windowHeight;
	auto wsStyle = isChildWindow ? (WS_CHILD | WS_VISIBLE) : WS_OVERLAPPEDWINDOW;
	if (!isChildWindow)
	{
		RECT desiredRect = {0, 0, (LONG)windowWidth, (LONG)windowHeight};
		AdjustWindowRectEx(&desiredRect, wsStyle, FALSE, WS_EX_APPWINDOW);
		adjustedWidth = desiredRect.right - desiredRect.left;
		adjustedHeight = desiredRect.bottom - desiredRect.top;
	}
	UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
	if (windowX == -1 && windowY == -1)
		flags |= SWP_NOMOVE;
	SetWindowPos(hwnd,
		HWND_TOPMOST,
		(windowX == -1 ? 0 : windowX),          // Use explicit or default X position
			(windowY == -1 ? 0 : windowY),
	adjustedWidth,
		adjustedHeight, flags);
};
anex::IWindow &GLWindow::createChildWindow(const char* title, IScene &scene, const uint32_t& windowWidth, const uint32_t& windowHeight, const int32_t& windowX, const int32_t& windowY)
{
	childWindows.emplace_back(*this, (GLScene &)scene, title, windowWidth, windowHeight, windowX, windowY);
	return childWindows.back();
};
void anex::modules::gl::computeNormals(const std::vector<uint32_t> &indices, const std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals)
{
	const glm::vec3 *positionsData = positions.data();
	const glm::ivec3 *indicesData = (const glm::ivec3 *)indices.data();
	const int nbTriangles = indices.size() / 3;
	const int nbVertices = positions.size();
	const int nbNormals = nbVertices;
	normals.resize(nbNormals);
	auto *normalsData = normals.data();
	memset(normalsData, 0, nbVertices * sizeof(glm::vec3));
	for (uint32_t index = 0; index < nbTriangles; index++)
	{
		const auto& indice = indicesData[index];
		const auto& vertex1 = positionsData[indice.x];
		const auto& vertex2 = positionsData[indice.y];
		const auto& vertex3 = positionsData[indice.z];
		auto& normal1 = normalsData[indice.x];
		auto& normal2 = normalsData[indice.y];
		auto& normal3 = normalsData[indice.z];
		auto triangleNormal = cross(vertex2 - vertex1, vertex3 - vertex1);
		if (!triangleNormal.x && !triangleNormal.y && !triangleNormal.z)
		{
			continue;
		}
		normal1 += triangleNormal;
		normal2 += triangleNormal;
		normal3 += triangleNormal;
	}
	for (uint32_t index = 0; index < nbVertices; index++)
	{
		auto& normal = normalsData[index];
		normal = glm::normalize(normal);
	}
};
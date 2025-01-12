#include <anex/modules/gl/GLWindow.hpp>
#include <stdexcept>
#include <iostream>
#include <anex/modules/gl/shaders/ShaderManager.hpp>
#include <anex/Logger.hpp>
using namespace anex::modules::gl;
#ifdef _WIN32
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
GLTest::GLTest(const char* title, const int& windowWidth, const int& windowHeight, const int& framerate):
	IWindow(windowWidth, windowHeight, framerate),
	title(title)
{
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
	run();
};

GLTest::~GLTest()
{
};
#ifdef _WIN32
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
typedef BOOL (APIENTRY * PFNWGLSWAPINTERVALEXTPROC) (int interval);
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;
static const uint8_t GL_KEYCODES[] = {0,27,49,50,51,52,53,54,55,56,57,48,45,61,8,9,81,87,69,82,84,89,85,73,79,80,91,93,10,0,65,83,68,70,71,72,74,75,76,59,39,96,0,92,90,88,67,86,66,78,77,44,46,47,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,17,3,0,20,0,19,0,5,18,4,26,127};
static LRESULT CALLBACK gl_wndproc(HWND hwnd, UINT msg, WPARAM wParam,
																	LPARAM lParam)
{
	struct GLTest* glWindow = (struct GLTest*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg)
	{
	case WM_CREATE:
		{
			CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
			glWindow = (GLTest*)createStruct->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)glWindow);
			glWindow->hDeviceContext = GetDC(hwnd);
			glWindow->renderInit();
			break;
		};
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		glWindow->windowButtons[0] = (msg == WM_LBUTTONDOWN);
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		glWindow->windowButtons[1] = (msg == WM_RBUTTONDOWN);
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		glWindow->windowButtons[2] = (msg == WM_MBUTTONDOWN);
		break;
		case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam); // This gives the scroll amount
			if (zDelta > 0)
			{
				glWindow->windowButtons[3] = 1; // Wheel scrolled up
			}
			else
			{
				glWindow->windowButtons[4] = 1; // Wheel scrolled down
			}
			break;
		};
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	{
		WORD button = GET_XBUTTON_WPARAM(wParam);
		if (button == XBUTTON2) // Back button
		{
			glWindow->windowButtons[5] = (msg == WM_XBUTTONDOWN);
		}
		else if (button == XBUTTON1) // Forward button
		{
			glWindow->windowButtons[6] = (msg == WM_XBUTTONDOWN);
		}
		break;
	};
	case WM_MOUSEMOVE:
		glWindow->mouseCoords.y = glWindow->windowHeight - HIWORD(lParam), glWindow->mouseCoords.x = LOWORD(lParam);
		glWindow->mouseMoved = true;
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		{
			glWindow->mod = ((GetKeyState(VK_CONTROL) & 0x8000) >> 15) |
							 ((GetKeyState(VK_SHIFT) & 0x8000) >> 14) |
							 ((GetKeyState(VK_MENU) & 0x8000) >> 13) |
							 (((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) >> 12);
			glWindow->windowKeys[GL_KEYCODES[HIWORD(lParam) & 0x1ff]] = !((lParam >> 31) & 1);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
#endif
void GLTest::startWindow()
{
#ifdef _WIN32
	hInstance = GetModuleHandle(NULL);
	WNDCLASSEX wc = {0};
	// wc.cbSize = sizeof(WNDCLASS);
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = gl_wndproc;
	wc.hInstance = hInstance;
	wc.lpszClassName = title;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClassEx(&wc);
	RECT desiredRect = {0, 0, windowWidth, windowHeight};
	AdjustWindowRectEx(&desiredRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);
	int adjustedWidth = desiredRect.right - desiredRect.left;
	int adjustedHeight = desiredRect.bottom - desiredRect.top;
	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, title, title,
												WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
												adjustedWidth, adjustedHeight, NULL, NULL, hInstance, this);

	if (hwnd == NULL)
		return;
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
	ShowWindow(hwnd, SW_NORMAL);
	UpdateWindow(hwnd);
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
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		GLcheck("glClearColor");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		GLcheck("glClear");
		render();
		SwapBuffers(hDeviceContext);
	}
#endif
_exit:
	scene.reset();
	shaders::ShaderManager::deleteShaders();
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

void GLTest::renderInit()
{
#ifdef _WIN32
	SetupPixelFormat(hDeviceContext);
	HGLRC hTempRC = wglCreateContext(hDeviceContext);
	wglMakeCurrent(hDeviceContext, hTempRC);
	gladLoadGL();
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
	gladLoadGL();
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	glEnable(GL_DEPTH_TEST);
	GLcheck("glEnable");
	glEnable(GL_CULL_FACE);
	GLcheck("glEnable");
	glCullFace(GL_BACK);
	GLcheck("glCullFace");
	glFrontFace(GL_CCW);
	GLcheck("glFrontFace");
	glViewport(0, 0, windowWidth, windowHeight);
	GLcheck("glViewport");
	glClearDepth(1.0);
	GLcheck("glClearDepth");
	glDepthRange(0.0, 1.0);
	GLcheck("glDepthRange");
	glEnable(GL_DEBUG_OUTPUT);
	GLcheck("glEnable");
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	GLcheck("glEnable");
	glDebugMessageCallback([](GLuint source, GLuint type, GLuint id, GLuint severity, GLsizei length, const GLchar* message, const void* userParam) {
		if (type == GL_DEBUG_TYPE_OTHER)
		{
			return;
		}
		std::cerr << "OpenGL Debug Message: " << message << std::endl;
	}, nullptr);
	wglSwapIntervalEXT(1);
#endif
};

void GLTest::updateKeyboard()
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

void GLTest::updateMouse()
{
	for (unsigned int i = 0; i < 7; ++i)
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

void GLTest::close()
{
#ifdef _WIN32
  PostMessage(hwnd, WM_CLOSE, 0, 0);
#endif
};
void GLTest::drawLine(int x0, int y0, int x1, int y1, uint32_t color)
{
};
void GLTest::drawRectangle(int x, int y, int w, int h, uint32_t color)
{
};
void GLTest::drawCircle(int x, int y, int radius, uint32_t color)
{
};
void GLTest::drawText(int x, int y, const char* text, int scale, uint32_t color)
{
};
const bool anex::modules::gl::GLcheck(const char* fn, const bool& egl)
{
	while (true)
	{
		uint32_t err = 0;
		if (!egl)
			err = glGetError();
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
void GLTest::warpPointer(const glm::vec2 &coords)
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
void GLTest::createChildWindow(const char *title, const uint32_t &windowWidth, const uint32_t &windowHeight)
{
	WNDCLASS childWc = {};
	childWc.lpfnWndProc = DefWindowProc; // Default procedure for the game window
	childWc.hInstance = hInstance;
	childWc.lpszClassName = title;
	RegisterClass(&childWc);
	HWND childWindow = CreateWindowEx(
		0,
		title,
		nullptr,
		WS_CHILD | WS_VISIBLE,
		0, 0, windowWidth, windowHeight,
		hwnd,
		nullptr,
		hInstance,
		nullptr
	);
	if (!childWindow)
	{
		throw std::runtime_error("Failed to create game window");
		return;
	}
	childWindows.push_back(childWindow);
};
#include <zg/modules/gl/RenderWindow.hpp>
#include <stdexcept>
#include <iostream>
#include <zg/modules/gl/shaders/ShaderManager.hpp>
#include <zg/Logger.hpp>
#include <zg/modules/gl/shaders/ShaderFactory.hpp>
#include <zg/modules/gl/textures/Texture.hpp>
#include <zg/modules/gl/entities/Plane.hpp>
using namespace zg::modules::gl;
#ifdef _WIN32
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
RenderWindow::RenderWindow(const char* title,
									 float windowWidth,
									 float windowHeight,
									 float windowX,
									 float windowY,
									 bool borderless,
									 uint32_t framerate):
	IWindow(windowWidth, windowHeight, windowX, windowY, borderless, framerate),
	title(title),
	glContext(new GladGLContext),
	shaderContext(new ShaderContext)
{
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
	run();
};
RenderWindow::RenderWindow(RenderWindow &parentWindow,
									 GLScene &parentScene,
									 const char *childTitle,
									 float childWindowWidth,
									 float childWindowHeight,
									 float childWindowX,
									 float childWindowY,
									 bool NDCFramebufferPlane,
									 uint32_t framerate):
	IWindow(childWindowWidth, childWindowHeight, childWindowX, childWindowY, false, framerate),
	title(childTitle),
	isChildWindow(true),
	parentWindow(&parentWindow),
	parentScene(&parentScene),
	glContext(parentWindow.glContext),
	shaderContext(parentWindow.shaderContext),
	NDCFramebufferPlane(NDCFramebufferPlane),
	framebufferTexture(std::make_shared<textures::Texture>(parentWindow, glm::ivec4(childWindowWidth, childWindowHeight, 1, 0), (void*)0)),
	framebufferDepthTexture(std::make_shared<textures::Texture>(parentWindow, glm::ivec4(childWindowWidth, childWindowHeight, 1, 0), (void*)0)),
	framebuffer(std::make_shared<textures::Framebuffer>(parentWindow, *framebufferTexture, *framebufferDepthTexture)),
	framebufferPlane(std::make_shared<entities::Plane>(
		parentWindow,
		parentScene,
		glm::vec3(
			NDCFramebufferPlane ?
				(-1 + ((childWindowX + (childWindowWidth / 2)) / parentWindow.windowWidth / 0.5)) :
				childWindowX + (childWindowWidth / 2),
			NDCFramebufferPlane ?
				(1 - ((childWindowY + (childWindowHeight / 2)) / parentWindow.windowHeight / 0.5)) :
				parentWindow.windowHeight - childWindowY - (childWindowHeight / 2), 0.2
		),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec2(
			childWindowWidth / (NDCFramebufferPlane ? (parentWindow.windowWidth / 2) : 1),
			childWindowHeight / (NDCFramebufferPlane ? (parentWindow.windowHeight / 2) : 1)
		),
		*framebufferTexture))
{
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
	framebufferPlane->addToBVH = false;
}
RenderWindow::~RenderWindow()
{
	if (!isChildWindow)
		delete glContext;
}
void RenderWindow::startWindow()
{
	iPlatformWindow = createPlatformWindow();
	auto &iPlatformWindowRef = *iPlatformWindow;
	iPlatformWindowRef.init(*this);
	iPlatformWindowRef.postInit();
	iPlatformWindowRef.loop();
_exit:
	scene.reset();
	delete shaderContext;
	for (auto &childWindowPointer : childWindows)
	{
		delete childWindowPointer;
	}
	childWindows.clear();
	iPlatformWindowRef.destroy();
}
void RenderWindow::updateKeyboard()
{
	for (unsigned int i = 0; i < 256; ++i)
	{
		auto& pressed = windowKeys[i];
		if (keys[i] != pressed)
		{
			callKeyPressHandler(i, pressed);
			callAnyKeyPressHandler(i, pressed);
		}
		if (pressed)
		{
			callKeyUpdateHandler(i);
		}
	}
};

void RenderWindow::updateMouse()
{
	for (unsigned int i = RenderWindow::MinMouseButton; i < RenderWindow::MaxMouseButton; ++i)
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

void RenderWindow::close()
{
#ifdef _WIN32
	if (isChildWindow)
	{
		return;
	}
	iPlatformWindow->close();
#endif
};
void RenderWindow::minimize()
{
#ifdef _WIN32
	minimized = true;
	maximized = false;
	if (isChildWindow)
	{
		return;
	}
	iPlatformWindow->minimize();
	buttons.clear();
	for (unsigned i = 0; i <= RenderWindow::MaxMouseButton; ++i)
	{
		windowButtons[i] = false;
	}
#endif
};
void RenderWindow::maximize()
{
#ifdef _WIN32
	minimized = false;
	if (maximized)
	{
		maximized = false;
		iPlatformWindow->restore();
		setXY(oldXY.x, oldXY.y);
	}
	else
	{
		maximized = true;
		iPlatformWindow->maximize();
		oldXY.x = windowX;
		oldXY.y = windowY;
		setXY(0, 0);
	}
#endif
};
void RenderWindow::restore()
{
#ifdef _WIN32
	minimized = false;
	maximized = false;
	if (isChildWindow)
	{
		return;
	}
	iPlatformWindow->restore();
	setXY(oldXY.x, oldXY.y);
#endif
}
void RenderWindow::preRender()
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
void RenderWindow::postRender()
{
	if (!isChildWindow)
	{
		return;
	}
	framebuffer->unbind();
};
void RenderWindow::drawLine(int x0, int y0, int x1, int y1, uint32_t color)
{
};
void RenderWindow::drawRectangle(int x, int y, int w, int h, uint32_t color)
{
};
void RenderWindow::drawCircle(int x, int y, int radius, uint32_t color)
{
};
void RenderWindow::drawText(int x, int y, const char* text, int scale, uint32_t color)
{
};
const bool zg::modules::gl::GLcheck(RenderWindow &window, const char* fn, const bool egl)
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
void RenderWindow::warpPointer(glm::vec2 coords)
{
	iPlatformWindow->warpPointer(coords);
	justWarpedPointer = true;
// #if defined(_UNIX)
// 	DEFINE_X11_WINDOW_DRIVER;
// 	XWarpPointer(x11WindowDriver.display, None, x11WindowDriver.window, 0, 0, 0, 0, position.x, position.y);
// #endif
};
void RenderWindow::setXY(float x, float y)
{
	windowX = x;
	windowY = y;
	if (isChildWindow)
	{
		return;
	}
	iPlatformWindow->setXY();
};
void RenderWindow::setWidthHeight(float width, float height)
{
	windowWidth = width;
	windowHeight = height;
	if (isChildWindow)
	{
		return;
	}
	iPlatformWindow->setWidthHeight();
};
void RenderWindow::mouseCapture(bool capture)
{
	iPlatformWindow->mouseCapture(capture);
}
zg::IWindow &RenderWindow::createChildWindow(const char* title,
																					 IScene &scene,
																					 float windowWidth,
																					 float windowHeight,
																					 float windowX,
																					 float windowY,
																					 bool NDCFramebufferPlane)
{
	childWindows.push_back(new RenderWindow(*this, (GLScene &)scene, title, windowWidth, windowHeight, windowX, windowY, NDCFramebufferPlane));
	return *childWindows.back();
};
void zg::modules::gl::computeNormals(const std::vector<uint32_t> &indices, const std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals)
{
	const glm::vec3 *positionsData = positions.data();
	const auto *indicesData = (const glm::ivec3 *)indices.data();
	const auto nbTriangles = indices.size() / 3;
	const auto nbVertices = positions.size();
	const auto nbNormals = nbVertices;
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
		if (!static_cast<bool>(triangleNormal.x) && !static_cast<bool>(triangleNormal.y) && !static_cast<bool>(triangleNormal.z))
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
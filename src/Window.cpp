#include <../include/zg/Logger.hpp>
#include <../include/zg/Window.hpp>
#include <../include/zg/entities/Plane.hpp>
#include <../include/zg/shaders/ShaderFactory.hpp>
#include <../include/zg/shaders/ShaderManager.hpp>
#include <../include/zg/textures/Texture.hpp>
#include <iostream>
#include <stdexcept>
using namespace zg;
#ifdef _WIN32
extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
Window::Window(const char *title,
			   float windowWidth,
			   float windowHeight,
			   float windowX,
			   float windowY,
			   bool borderless,
			   bool _vsync,
			   uint32_t framerate) : IWindow(windowWidth, windowHeight, windowX, windowY, borderless, framerate),
									 title(title),
									 shaderContext(new ShaderContext),
									 vsync(_vsync)
{
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
}
Window::Window(Window &_parentWindow,
			   Scene &_parentScene,
			   const char *_childTitle,
			   float childWindowWidth,
			   float childWindowHeight,
			   float childWindowX,
			   float childWindowY,
			   bool _NDCFramebufferPlane,
			   bool _vsync,
			   uint32_t framerate) : IWindow(childWindowWidth, childWindowHeight, childWindowX, childWindowY, false, framerate),
									 iRenderer(_parentWindow.iRenderer),
									 title(_childTitle),
									 isChildWindow(true),
									 parentWindow(&_parentWindow),
									 parentScene(&_parentScene),
									 shaderContext(_parentWindow.shaderContext),
									 NDCFramebufferPlane(_NDCFramebufferPlane),
									 framebufferTexture(std::make_shared<textures::Texture>(_parentWindow, glm::ivec4(childWindowWidth, childWindowHeight, 1, 0), (void *)0)),
									 framebufferDepthTexture(std::make_shared<textures::Texture>(_parentWindow, glm::ivec4(childWindowWidth, childWindowHeight, 1, 0), (void *)0)),
									 framebuffer(std::make_shared<textures::Framebuffer>(_parentWindow, *framebufferTexture, *framebufferDepthTexture)),
									 framebufferPlane(std::make_shared<entities::Plane>(
										 _parentWindow,
										 *parentScene,
										 glm::vec3(
											 NDCFramebufferPlane ? (-1 + ((childWindowX + (childWindowWidth / 2)) / _parentWindow.windowWidth / 0.5)) : childWindowX + (childWindowWidth / 2),
											 NDCFramebufferPlane ? (1 - ((childWindowY + (childWindowHeight / 2)) / _parentWindow.windowHeight / 0.5)) : _parentWindow.windowHeight - childWindowY - (childWindowHeight / 2), 0.2),
										 glm::vec3(0),
										 glm::vec3(1),
										 glm::vec2(
											 childWindowWidth / (NDCFramebufferPlane ? (_parentWindow.windowWidth / 2) : 1),
											 childWindowHeight / (NDCFramebufferPlane ? (_parentWindow.windowHeight / 2) : 1)),
										 *framebufferTexture)),
									 vsync(_vsync)
{
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
	framebufferPlane->addToBVH = false;
}
void Window::startWindow()
{
	iPlatformWindow = createPlatformWindow();
	auto &iPlatformWindowRef = *iPlatformWindow;
	iRenderer = createRenderer();
	auto &iRendererRef = *iRenderer;
	iPlatformWindowRef.init(*this);
	iPlatformWindowRef.pollMessages();
	iRendererRef.createContext(&iPlatformWindowRef);
	iRendererRef.init();
	iPlatformWindowRef.postInit();
	runRunnables();
	while (iPlatformWindowRef.pollMessages())
	{
		runRunnables();
		updateKeyboard();
		updateMouse();
		iRendererRef.preBeginRenderPass();
		update();
		for (auto &childWindowPointer : childWindows)
		{
			auto &childWindow = *childWindowPointer;
			if (childWindow.minimized)
				continue;
			childWindow.render();
		}
		iRendererRef.beginRenderPass();
		render();
		for (auto &childWindowPointer : childWindows)
		{
			auto &childWindow = *childWindowPointer;
			if (childWindow.minimized)
				continue;
			childWindow.framebufferPlane->render();
		}
		iRendererRef.postRenderPass();
		iRendererRef.swapBuffers();
	}
_exit:
	scene.reset();
	delete shaderContext;
	for (auto &childWindowPointer : childWindows)
	{
		delete childWindowPointer;
	}
	childWindows.clear();
	iRendererRef.destroy();
	iPlatformWindowRef.destroy();
	delete iRenderer;
	delete iPlatformWindow;
}
void Window::updateKeyboard()
{
	for (unsigned int i = 0; i < 256; ++i)
	{
		auto &pressed = windowKeys[i];
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

void Window::updateMouse()
{
	for (unsigned int i = Window::MinMouseButton; i < Window::MaxMouseButton; ++i)
	{
	_checkPressed:
		auto &pressed = windowButtons[i];
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
}
void Window::close()
{
	if (isChildWindow)
	{
		return;
	}
	iPlatformWindow->close();
}
void Window::minimize()
{
	minimized = true;
	maximized = false;
	if (isChildWindow)
	{
		return;
	}
	iPlatformWindow->minimize();
	buttons.clear();
	for (unsigned i = 0; i <= Window::MaxMouseButton; ++i)
	{
		windowButtons[i] = false;
	}
}
void Window::maximize()
{
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
}
void Window::restore()
{
	minimized = false;
	maximized = false;
	if (isChildWindow)
	{
		return;
	}
	iPlatformWindow->restore();
	setXY(oldXY.x, oldXY.y);
}
void Window::preRender()
{
	if (!isChildWindow)
	{
		return;
	}
	runRunnables();
	updateKeyboard();
	updateMouse();
	framebuffer->bind();
}
void Window::postRender()
{
	if (!isChildWindow)
	{
		return;
	}
	framebuffer->unbind();
}
void Window::drawLine(int x0, int y0, int x1, int y1, uint32_t color)
{
}
void Window::drawRectangle(int x, int y, int w, int h, uint32_t color)
{
}
void Window::drawCircle(int x, int y, int radius, uint32_t color)
{
}
void Window::drawText(int x, int y, const char *text, int scale, uint32_t color)
{
}
void Window::warpPointer(glm::vec2 coords)
{
	iPlatformWindow->warpPointer(coords);
	justWarpedPointer = true;
}
void Window::setXY(float x, float y)
{
	windowX = x;
	windowY = y;
	if (isChildWindow)
	{
		return;
	}
	iPlatformWindow->setXY();
}
void Window::setWidthHeight(float width, float height)
{
	windowWidth = width;
	windowHeight = height;
	if (isChildWindow)
	{
		return;
	}
	iPlatformWindow->setWidthHeight();
}
void Window::mouseCapture(bool capture)
{
	iPlatformWindow->mouseCapture(capture);
}
zg::Window &Window::createChildWindow(const char *title,
									  IScene &scene,
									  float windowWidth,
									  float windowHeight,
									  float windowX,
									  float windowY,
									  bool NDCFramebufferPlane)
{
	childWindows.push_back(new Window(*this, (Scene &)scene, title, windowWidth, windowHeight, windowX, windowY, NDCFramebufferPlane));
	return *childWindows.back();
}
void zg::computeNormals(const std::vector<uint32_t> &indices, const std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals)
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
		const auto &indice = indicesData[index];
		const auto &vertex1 = positionsData[indice.x];
		const auto &vertex2 = positionsData[indice.y];
		const auto &vertex3 = positionsData[indice.z];
		auto &normal1 = normalsData[indice.x];
		auto &normal2 = normalsData[indice.y];
		auto &normal3 = normalsData[indice.z];
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
		auto &normal = normalsData[index];
		normal = glm::normalize(normal);
	}
}
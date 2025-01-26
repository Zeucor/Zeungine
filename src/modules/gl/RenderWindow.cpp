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
	iVendorRenderer(parentWindow.iVendorRenderer),
	title(childTitle),
	isChildWindow(true),
	parentWindow(&parentWindow),
	parentScene(&parentScene),
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
void RenderWindow::startWindow()
{
	iPlatformWindow = createPlatformWindow();
	auto &iPlatformWindowRef = *iPlatformWindow;
	iPlatformWindowRef.init(*this);
	iVendorRenderer = createVendorRenderer();
	iPlatformWindowRef.createContext();
	iVendorRenderer->init(&iPlatformWindowRef);
	auto &iVendorRendererRef = *iVendorRenderer;
	iPlatformWindowRef.postInit();
	while (iPlatformWindowRef.pollMessages())
	{
		runRunnables();
		updateKeyboard();
		updateMouse();
		for (auto &childWindowPointer : childWindows)
		{
			auto &childWindow = *childWindowPointer;
			if (childWindow.minimized)
				continue;
			childWindow.render();
		}
		iVendorRendererRef.render();
	}
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
}
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
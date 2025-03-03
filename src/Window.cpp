#include <iostream>
#include <stdexcept>
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <zg/system/Budget.hpp>
#include <zg/textures/Texture.hpp>
using namespace zg;
NANOSECONDS_DURATION windowSD = NANOSECONDS_DURATION((1.0 / 144.0) * NANOSECONDS::den);
budget::ZBudget windowBudget(windowSD);
#ifdef _WIN32
extern "C" {
_declspec(dllexport) DWORD NvOptimusEnablement = 1;
_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
Window::Window(const char* title, float windowWidth, float windowHeight, float windowX, float windowY, bool borderless,
							 bool _vsync, uint32_t framerate) :
		windowWidth(windowWidth), windowHeight(windowHeight), windowX(windowX), windowY(windowY), borderless(borderless),
		framerate(framerate), title(title), shaderContext(new ShaderContext), vsync(_vsync)
{
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
}
Window::Window(Window& _parentWindow, Scene& _parentScene, const char* _childTitle, float childWindowWidth,
							 float childWindowHeight, float childWindowX, float childWindowY, bool _NDCFramebufferPlane, bool _vsync,
							 uint32_t framerate) :
		iRenderer(_parentWindow.iRenderer), windowWidth(childWindowWidth), windowHeight(childWindowHeight),
		windowX(childWindowX), windowY(childWindowY), framerate(framerate), borderless(false), title(_childTitle),
		isChildWindow(true), parentWindow(&_parentWindow), parentScene(&_parentScene),
		shaderContext(_parentWindow.shaderContext), NDCFramebufferPlane(_NDCFramebufferPlane),
		framebufferTexture(std::make_shared<textures::Texture>(
			_parentWindow, glm::ivec4(childWindowWidth, childWindowHeight, 1, 0), (void*)0)),
		framebufferDepthTexture(std::make_shared<textures::Texture>(
			_parentWindow, glm::ivec4(childWindowWidth, childWindowHeight, 1, 0), (void*)0)),
		framebuffer(std::make_shared<textures::Framebuffer>(_parentWindow, *framebufferTexture, *framebufferDepthTexture)),
		framebufferPlane(std::make_shared<entities::Plane>(
			_parentWindow, *parentScene,
			glm::vec3(NDCFramebufferPlane ? (-1 + ((childWindowX + (childWindowWidth / 2)) / _parentWindow.windowWidth / 0.5))
																		: childWindowX + (childWindowWidth / 2),
								NDCFramebufferPlane
									? (1 - ((childWindowY + (childWindowHeight / 2)) / _parentWindow.windowHeight / 0.5))
									: _parentWindow.windowHeight - childWindowY - (childWindowHeight / 2),
								0.2),
			glm::vec3(0), glm::vec3(1),
			glm::vec2(childWindowWidth / (NDCFramebufferPlane ? (_parentWindow.windowWidth / 2) : 1),
								childWindowHeight / (NDCFramebufferPlane ? (_parentWindow.windowHeight / 2) : 1)),
			*framebufferTexture)),
		vsync(_vsync)
{
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
	framebufferPlane->addToBVH = false;
}
void Window::run()
{
#if defined(_WIN32) || defined(__linux__)
	windowThread = std::make_shared<std::thread>(&Window::startWindow, this);
	windowThread->join();
#elif defined(MACOS)
	startWindow();
#endif
}
void Window::update()
{
	if (scene)
		scene->update();
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
	auto& framebufferRef = *framebuffer;
	framebufferRef.scenePointer = (Scene*)scene.get();
	framebufferRef.bind();
}
void Window::render()
{
	std::lock_guard lock(renderMutex);
	preRender();
	updateDeltaTime();
	if (scene)
		scene->render();
	postRender();
};
void Window::postRender()
{
	if (!isChildWindow)
	{
		return;
	}
	framebuffer->unbind();
}
void Window::startWindow()
{
	iPlatformWindow = createPlatformWindow();
	auto& iPlatformWindowRef = *iPlatformWindow;
	iRenderer = createRenderer();
	auto& iRendererRef = *iRenderer;
	iPlatformWindowRef.init(*this);
	iPlatformWindowRef.pollMessages();
	iRendererRef.createContext(&iPlatformWindowRef);
	iRendererRef.init();
	iPlatformWindowRef.postInit();
	runRunnables();
	while (true)
	{
		windowBudget.begin();
		if (!iPlatformWindowRef.pollMessages())
		{
			windowBudget.sleep();
			break;
		}
		windowBudget.tick();
		iRendererRef.preBeginRenderPass();
		windowBudget.tick();
		runRunnables();
		windowBudget.tick();
		updateKeyboard();
		windowBudget.tick();
		updateMouse();
		windowBudget.tick();
		update();
		windowBudget.tick();
		for (auto& childWindowPointer : childWindows)
		{
			auto& childWindow = *childWindowPointer;
			if (childWindow.minimized)
				continue;
			windowBudget.tick();
			childWindow.render();
		}
		windowBudget.tick();
		iRendererRef.beginRenderPass();
		windowBudget.tick();
		render();
		windowBudget.tick();
		for (auto& childWindowPointer : childWindows)
		{
			auto& childWindow = *childWindowPointer;
			if (childWindow.minimized)
				continue;
			windowBudget.tick();
			childWindow.framebufferPlane->render();
		}
		windowBudget.tick();
		iRendererRef.postRenderPass();
		windowBudget.tick();
		callPreSwapbuffersOnceoff();
		iRendererRef.swapBuffers();
		windowBudget.end();
		windowBudget.sleep();
	}
_exit:
	audioEngine.stop();
	delete shaderContext;
	for (auto& childWindowPointer : childWindows)
	{
		delete childWindowPointer;
	}
	childWindows.clear();
	scene.reset();
	iRendererRef.destroy();
	iPlatformWindowRef.destroy();
	delete iRenderer;
	delete iPlatformWindow;
}
void Window::updateKeyboard()
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
}
void Window::updateMouse()
{
	for (unsigned int i = MinMouseButton; i < MaxMouseButton; ++i)
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
	for (unsigned i = 0; i <= MaxMouseButton; ++i)
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
void Window::mouseCapture(bool capture) { iPlatformWindow->mouseCapture(capture); }
zg::Window& Window::createChildWindow(const char* title, Scene& scene, float windowWidth, float windowHeight,
																			float windowX, float windowY, bool NDCFramebufferPlane)
{
	childWindows.push_back(
		new Window(*this, (Scene&)scene, title, windowWidth, windowHeight, windowX, windowY, NDCFramebufferPlane));
	return *childWindows.back();
}

// Keyboard
EventIdentifier Window::addKeyPressHandler(Key key, const KeyPressHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyPressHandlers[key];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void Window::removeKeyPressHandler(Key key, EventIdentifier& id)
{
	std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyPressHandlers[key];
	auto handlerIter = handlersPair.second.find(id);
	if (handlerIter == handlersPair.second.end())
	{
		return;
	}
	handlersPair.second.erase(handlerIter);
	id = 0;
};
EventIdentifier Window::addKeyUpdateHandler(Key key, const KeyUpdateHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyUpdateHandlers[key];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void Window::removeKeyUpdateHandler(Key key, EventIdentifier& id)
{
	std::lock_guard lock(handlersMutex);
	auto handlersIter = keyUpdateHandlers.find(key);
	if (handlersIter == keyUpdateHandlers.end())
		return;
	auto& handlers = handlersIter->second.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void Window::callKeyPressHandler(Key key, int pressed)
{
	keys[key] = pressed;
	{
		std::vector<KeyPressHandler> handlersCopy;
		{
			std::lock_guard lock(handlersMutex);
			auto handlersIter = keyPressHandlers.find(key);
			if (handlersIter == keyPressHandlers.end())
				return;
			auto& handlersMap = handlersIter->second.second;
			for (const auto& pair : handlersMap)
				handlersCopy.push_back(pair.second);
		}
		for (auto& handler : handlersCopy)
		{
			handler(!!pressed);
		}
	}
};
void Window::callKeyUpdateHandler(Key key)
{
	std::vector<KeyUpdateHandler> handlersCopy;
	{
		std::lock_guard lock(handlersMutex);
		auto handlersIter = keyUpdateHandlers.find(key);
		if (handlersIter == keyUpdateHandlers.end())
			return;
		auto& handlersMap = handlersIter->second.second;
		for (const auto& pair : handlersMap)
			handlersCopy.push_back(pair.second);
	}
	for (auto& handler : handlersCopy)
	{
		handler();
	}
};
EventIdentifier Window::addAnyKeyPressHandler(const AnyKeyPressHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto id = ++anyKeyPressHandlers.first;
	anyKeyPressHandlers.second[id] = callback;
	return id;
};
void Window::removeAnyKeyPressHandler(EventIdentifier& id)
{
	std::lock_guard lock(handlersMutex);
	auto& handlers = anyKeyPressHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void Window::callAnyKeyPressHandler(Key key, bool pressed)
{
	std::vector<AnyKeyPressHandler> handlersCopy;
	{
		std::lock_guard lock(handlersMutex);
		auto& handlersMap = anyKeyPressHandlers.second;
		for (const auto& pair : handlersMap)
			handlersCopy.push_back(pair.second);
	}
	for (auto& handler : handlersCopy)
	{
		handler(key, pressed);
	}
}
void Window::handleKey(Key key, int32_t mod, bool pressed)
{
	auto& window = *dynamic_cast<Window*>(this);
	bool hadChildFocus = false;
	for (auto& childWindowPointer : window.childWindows)
	{
		auto& childWindow = *childWindowPointer;
		if (childWindow.minimized)
			continue;
		if (!childWindow.focused)
			continue;
		childWindow.mod = mod;
		childWindow.windowKeys[key] = pressed;
		hadChildFocus = true;
		break;
	}
	if (!hadChildFocus)
	{
		window.mod = mod;
		window.windowKeys[key] = pressed;
	}
}
// Mouse
EventIdentifier Window::addMousePressHandler(Button button, const MousePressHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto& handlersPair = mousePressHandlers[button];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void Window::removeMousePressHandler(Button button, EventIdentifier& id)
{
	std::lock_guard lock(handlersMutex);
	auto& handlersPair = mousePressHandlers[button];
	auto handlerIter = handlersPair.second.find(id);
	if (handlerIter == handlersPair.second.end())
	{
		return;
	}
	handlersPair.second.erase(handlerIter);
	id = 0;
};
EventIdentifier Window::addMouseMoveHandler(const MouseMoveHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto id = ++mouseMoveHandlers.first;
	mouseMoveHandlers.second[id] = callback;
	return id;
};
void Window::removeMouseMoveHandler(EventIdentifier& id)
{
	std::lock_guard lock(handlersMutex);
	auto& handlers = mouseMoveHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void Window::callMousePressHandler(Button button, int pressed)
{
	buttons[button] = pressed;
	{
		std::vector<MousePressHandler> handlersCopy;
		{
			std::lock_guard lock(handlersMutex);
			auto handlersIter = mousePressHandlers.find(button);
			if (handlersIter == mousePressHandlers.end())
				return;
			auto& handlersMap = handlersIter->second.second;
			for (const auto& pair : handlersMap)
				handlersCopy.push_back(pair.second);
		}
		for (auto& handler : handlersCopy)
		{
			handler(!!pressed);
		}
	}
}
void Window::callMouseMoveHandler(glm::vec2 coords)
{
	std::vector<MouseMoveHandler> handlersCopy;
	{
		std::lock_guard lock(handlersMutex);
		auto& handlersMap = mouseMoveHandlers.second;
		for (const auto& pair : handlersMap)
			handlersCopy.push_back(pair.second);
	}
	for (auto& handler : handlersCopy)
	{
		handler(coords);
	}
}
void Window::handleMouseMove(uint32_t x, uint32_t y)
{
	auto& window = *dynamic_cast<Window*>(this);
	bool hadChildFocus = false;
	for (auto& childWindowPointer : window.childWindows)
	{
		auto& childWindow = *childWindowPointer;
		if (childWindow.minimized)
			continue;
		if (!childWindow.focused)
			continue;
		auto childX = x - childWindow.windowX;
		auto childY = childWindow.windowHeight - (window.windowHeight - y - childWindow.windowY);
		childWindow.mouseCoords.x = childX, childWindow.mouseCoords.y = childY;
		childWindow.mouseMoved = true;
		hadChildFocus = true;
		break;
	}
	if (!hadChildFocus)
	{
		window.mouseCoords.y = y, window.mouseCoords.x = x;
		window.mouseMoved = true;
	}
}
void Window::handleMousePress(Button button, bool pressed)
{
	auto& window = *dynamic_cast<Window*>(this);
	bool hadChildFocus = false;
	for (auto& childWindowPointer : window.childWindows)
	{
		auto& childWindow = *childWindowPointer;
		if (childWindow.minimized)
			continue;
		if (!childWindow.focused)
			continue;
		childWindow.windowButtons[button] = pressed;
		hadChildFocus = true;
		break;
	}
	if (!hadChildFocus)
		window.windowButtons[button] = pressed;
}
// resize
EventIdentifier Window::addResizeHandler(const ViewResizeHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto id = ++viewResizeHandlers.first;
	viewResizeHandlers.second[id] = callback;
	return id;
};
void Window::removeResizeHandler(EventIdentifier& id)
{
	std::lock_guard lock(handlersMutex);
	auto& handlers = viewResizeHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void Window::callResizeHandler(glm::vec2 newSize)
{
	std::vector<ViewResizeHandler> handlersCopy;
	{
		std::lock_guard lock(handlersMutex);
		auto& handlersMap = viewResizeHandlers.second;
		for (const auto& pair : handlersMap)
			handlersCopy.push_back(pair.second);
	}
	for (auto& handler : handlersCopy)
	{
		handler(newSize);
	}
};
// focus
EventIdentifier Window::addFocusHandler(const FocusHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto id = ++focusHandlers.first;
	focusHandlers.second[id] = callback;
	return id;
}
void Window::removeFocusHandler(EventIdentifier& id)
{
	std::lock_guard lock(handlersMutex);
	auto& handlers = focusHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
}
void Window::callFocusHandler(bool focused)
{
	if (this->focused == focused)
		return;
	std::vector<FocusHandler> handlersCopy;
	{
		std::lock_guard lock(handlersMutex);
		auto& handlersMap = focusHandlers.second;
		for (const auto& pair : handlersMap)
			handlersCopy.push_back(pair.second);
	}
	this->focused = focused;
	for (auto& handler : handlersCopy)
	{
		handler(focused);
	}
}
// onceoffs
void Window::addPreSwapbuffersOnceoff(const PreSwapbuffersOnceoff& onceoff)
{
	std::lock_guard lock(handlersMutex);
	preSwapbuffersOnceoffs.push(onceoff);
}
void Window::callPreSwapbuffersOnceoff()
{
	handlersMutex.lock();
	if (preSwapbuffersOnceoffs.empty())
	{
		handlersMutex.unlock();
		return;
	}
	auto onceoff = preSwapbuffersOnceoffs.front();
	preSwapbuffersOnceoffs.pop();
	handlersMutex.unlock();
	onceoff();
}
std::shared_ptr<Scene> Window::setScene(const std::shared_ptr<Scene>& scene)
{
	this->scene = scene;
	return scene;
};
void Window::runOnThread(const Runnable& runnable)
{
	// std::lock_guard lock(runnablesMutex);
	runnables.push(runnable);
};
void Window::runRunnables()
{
	std::queue<Runnable> runnablesCopy;
	{
		// std::lock_guard lock(runnablesMutex);
		runnablesCopy = runnables;
		while (!runnables.empty())
			runnables.pop();
	}
	while (!runnablesCopy.empty())
	{
		auto runnable = runnablesCopy.front();
		runnablesCopy.pop();
		runnable(dynamic_cast<Window&>(*this));
	}
};
void Window::updateDeltaTime()
{
	auto currentTime = std::chrono::steady_clock::now();
	std::chrono::duration<long double> duration = currentTime - lastFrameTime;
	deltaTime = duration.count();
	lastFrameTime = currentTime;
};
void Window::resize(glm::vec2 newSize)
{
	if (windowWidth != newSize.x)
		windowWidth = newSize.x;
	if (windowHeight != newSize.y)
		windowHeight = newSize.y;
	if (scene)
		scene->resize(newSize);
	callResizeHandler(newSize);
};
void Window::registerOnEntityAddedFunction(const OnEntityAddedFunction& function)
{
	onEntityAdded = function;
	return;
}
void zg::computeNormals(const std::vector<uint32_t>& indices, const std::vector<glm::vec3>& positions,
												std::vector<glm::vec3>& normals)
{
	const glm::vec3* positionsData = positions.data();
	const auto* indicesData = (const glm::ivec3*)indices.data();
	const auto nbTriangles = indices.size() / 3;
	const auto nbVertices = positions.size();
	const auto nbNormals = nbVertices;
	normals.resize(nbNormals);
	auto* normalsData = normals.data();
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
		if (!static_cast<bool>(triangleNormal.x) && !static_cast<bool>(triangleNormal.y) &&
				!static_cast<bool>(triangleNormal.z))
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
}

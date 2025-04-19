#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <zg/textures/Texture.hpp>
using namespace zg;
#ifdef _WIN32
extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
Window::Window(const WindowCreateInfo& info):
	title(info.title),
	childWindows([](auto& childWindow) { return childWindow.title; }),
	windowWidth(info.windowWidth),
	windowHeight(info.windowHeight),
	windowX(info.windowX),
	windowY(info.windowY),
	scenes([](auto& scene) { return scene.name; }),
	deltaTime(1.0 / info.framerate),
	borderless(info.borderless),
	framerate(info.framerate),
	vsync(info.vsync), frameduration(NANOSECONDS_DURATION(deltaTime * NANOSECONDS::den)),
	framebudget(frameduration), systemFonts(*this)
{
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
	if (info.isChildWindow)
	{
		shaderContext = (ShaderContext*)info.parentWindowPointer->shaderContext;
		NDCFramebufferPlane = info.NDCFramebufferPlane;
		framebufferTexture = std::make_shared<textures::Texture>(
			info.parentWindowPointer->iRenderer, glm::ivec4(info.windowWidth, info.windowHeight, 1, 0), (void*)0);
		framebufferDepthTexture = std::make_shared<textures::Texture>(
			info.parentWindowPointer->iRenderer, glm::ivec4(info.windowWidth, info.windowHeight, 1, 0), (void*)0);
		framebuffer = std::make_shared<textures::Framebuffer>(
			*info.parentWindowPointer,
			std::vector<textures::Framebuffer::TextureAttachmentPair>(
				{{framebufferTexture.get(),
					textures::Framebuffer::AttachmentType::
						Color} /*, {framebufferDepthTexture.get(), textures::Framebuffer::AttachmentType::Depth}*/}));
		framebufferPlane->addToBVH = false;
	}
	else
	{
		shaderContext = new ShaderContext;
	}
}
Window::Window(const Window& other):
	title(other.title),
	childWindows(other.childWindows),
	windowWidth(other.windowWidth),
	windowHeight(other.windowHeight),
	windowX(other.windowX),
	windowY(other.windowY),
	scenes(other.scenes),
	deltaTime(1.0 / other.framerate),
	borderless(other.borderless),
	framerate(other.framerate),
	vsync(other.vsync), frameduration(NANOSECONDS_DURATION(deltaTime * NANOSECONDS::den)),
	framebudget(frameduration), systemFonts(*this)
{
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
	shaderContext = new ShaderContext;
}
Window& Window::operator=(const Window& other)
{
	iPlatformWindow = other.iPlatformWindow;
	iRenderer = other.iRenderer;
	windowWidth = other.windowWidth;
	windowHeight = other.windowHeight;
	windowX = other.windowX;
	windowY = other.windowY;
	framerate = other.framerate;
#if defined(_WIN32) || defined(__linux__)
	windowThread = other.windowThread;
#endif
	runnables = other.runnables;
	keys = other.keys;
	buttons = other.buttons;
	keyPressHandlers = other.keyPressHandlers;
	keyUpdateHandlers = other.keyUpdateHandlers;
	anyKeyPressHandlers = other.anyKeyPressHandlers;
	mousePressHandlers = other.mousePressHandlers;
	mouseMoveHandlers = other.mouseMoveHandlers;
	viewResizeHandlers = other.viewResizeHandlers;
	focusHandlers = other.focusHandlers;
	preSwapbuffersOnceoffs = other.preSwapbuffersOnceoffs;
	scenes = other.scenes;
	open = other.open;
	lastFrameTime = other.lastFrameTime;
	deltaTime = other.deltaTime;
	lastFrameDeltaTime = other.lastFrameDeltaTime;
	justWarpedPointer = other.justWarpedPointer;
	borderless = other.borderless;
	minimized = other.minimized;
	maximized = other.maximized;
	focused = other.focused;
	onEntityAdded = other.onEntityAdded;
	title = other.title;
	mouseMoved = other.mouseMoved;
	mouseCoords = other.mouseCoords;
	mod = other.mod;
	isChildWindow = other.isChildWindow;
	parentWindow = other.parentWindow;
	parentScene = other.parentScene;
	childWindows = other.childWindows;
	shaderContext = other.shaderContext;
	NDCFramebufferPlane = other.NDCFramebufferPlane;
	framebufferTexture = other.framebufferTexture;
	framebufferDepthTexture = other.framebufferDepthTexture;
	framebuffer = other.framebuffer;
	framebufferPlane = other.framebufferPlane;
	oldXY = other.oldXY;
	vsync = other.vsync;
	frameduration = other.frameduration;
	return *this;
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
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
		scenesData[index].update();
}
void Window::preRender()
{
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
		scenesData[index].preRender();
	if (!isChildWindow)
		return;
	runRunnables();
	updateKeyboard();
	updateMouse();
	auto& framebufferRef = *framebuffer;
	// framebufferRef.scenePointer = (Scene*)scene.get();
	framebufferRef.bind();
}
void Window::render()
{
	std::lock_guard lock(renderMutex);
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
		scenesData[index].render();
};
void Window::postRender()
{
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
		scenesData[index].postRender();
	if (!isChildWindow)
		return;
	framebuffer->unbind();
}
void Window::startWindow()
{
	iPlatformWindow = createPlatformWindow();
	auto& iPlatformWindowRef = *iPlatformWindow;
	iRenderer = createRenderer();
	auto& iRendererRef = *iRenderer;
	iPlatformWindowRef.init(*this);
	iRendererRef.createContext(&iPlatformWindowRef);
	iRendererRef.init();
	iPlatformWindowRef.postInit();
	runRunnables();
	iPlatformWindowRef.disableKeyAutoRepeat();
	while (true)
	{
		auto _now = framebudget.begin();
		updateDeltaTime(_now, false);
		if (!iPlatformWindowRef.pollMessages())
		{
			framebudget.sleep();
			break;
		}
		framebudget.tick();
		iRendererRef.preBeginRenderPass();
		framebudget.tick();
		runRunnables();
		framebudget.tick();
		updateKeyboard();
		framebudget.tick();
		updateMouse();
		framebudget.tick();
		update();
		framebudget.tick();
		auto childWindowsSize = childWindows.size();
		auto childWindowsData = childWindows.data();
		for (size_t index = 0; index < childWindowsSize; ++index)
		{
			auto& childWindow = childWindowsData[index];
			if (childWindow.minimized)
				continue;
			childWindow.render();
			framebudget.tick();
		}
		preRender();
		framebudget.tick();
		iRendererRef.beginRenderPass();
		framebudget.tick();
		render();
		framebudget.tick();
		for (size_t index = 0; index < childWindowsSize; ++index)
		{
			auto& childWindow = childWindowsData[index];
			if (childWindow.minimized)
				continue;
			framebudget.tick();
			childWindow.framebufferPlane->render();
		}
		framebudget.tick();
		iRendererRef.postRenderPass();
		framebudget.tick();
		postRender();
		framebudget.tick();
		callPreSwapbuffersOnceoff();
		framebudget.tick();
		_now = framebudget.end();
		updateDeltaTime(_now, true);
		iRendererRef.swapBuffers();
		framebudget.sleep();
	}
_exit:
	iPlatformWindowRef.enableKeyAutoRepeat();
	audioEngine.stop();
	audioEngine.clearPipeline();
	delete shaderContext;
	childWindows.clear();
	scenes.clear();
	iRendererRef.destroy();
	iPlatformWindowRef.destroy();
	delete iRenderer;
	delete iPlatformWindow;
	zg::Entity::cleanupSerialize();
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
zg::Window& Window::createChildWindow(const WindowCreateInfo& info)
{
	auto usingInfo{info};
	usingInfo.isChildWindow = true;
	return *std::get<KEY_ID_VECTOR_VALUE_INDEX>(childWindows.emplace_back(usingInfo));
}

// Keyboard
UniqueIdentifier Window::addKeyPressHandler(Key key, const KeyPressHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyPressHandlers[key];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void Window::removeKeyPressHandler(Key key, UniqueIdentifier& id)
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
UniqueIdentifier Window::addKeyUpdateHandler(Key key, const KeyUpdateHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyUpdateHandlers[key];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void Window::removeKeyUpdateHandler(Key key, UniqueIdentifier& id)
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
UniqueIdentifier Window::addAnyKeyPressHandler(const AnyKeyPressHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto id = ++anyKeyPressHandlers.first;
	anyKeyPressHandlers.second[id] = callback;
	return id;
};
void Window::removeAnyKeyPressHandler(UniqueIdentifier& id)
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
	auto childWindowsSize = childWindows.size();
	auto childWindowsData = childWindows.data();
	for (size_t index = 0; index < childWindowsSize; ++index)
	{
		auto& childWindow = childWindowsData[index];
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
UniqueIdentifier Window::addMousePressHandler(Button button, const MousePressHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto& handlersPair = mousePressHandlers[button];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void Window::removeMousePressHandler(Button button, UniqueIdentifier& id)
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
UniqueIdentifier Window::addMouseMoveHandler(const MouseMoveHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto id = ++mouseMoveHandlers.first;
	mouseMoveHandlers.second[id] = callback;
	return id;
};
void Window::removeMouseMoveHandler(UniqueIdentifier& id)
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
	auto childWindowsSize = childWindows.size();
	auto childWindowsData = childWindows.data();
	for (size_t index = 0; index < childWindowsSize; ++index)
	{
		auto& childWindow = childWindowsData[index];
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
	auto childWindowsSize = childWindows.size();
	auto childWindowsData = childWindows.data();
	for (size_t index = 0; index < childWindowsSize; ++index)
	{
		auto& childWindow = childWindowsData[index];
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
UniqueIdentifier Window::addResizeHandler(const ViewResizeHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto id = ++viewResizeHandlers.first;
	viewResizeHandlers.second[id] = callback;
	return id;
};
void Window::removeResizeHandler(UniqueIdentifier& id)
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
UniqueIdentifier Window::addFocusHandler(const FocusHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto id = ++focusHandlers.first;
	focusHandlers.second[id] = callback;
	return id;
}
void Window::removeFocusHandler(UniqueIdentifier& id)
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
Scene& Window::addScene(const SceneCreateInfo& info)
{
	auto usingInfo{info};
	usingInfo.windowPointer = this;
	auto scene_tuple = scenes.emplace_back(usingInfo);
	auto& scene = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(scene_tuple);
	auto ID = std::get<KEY_ID_VECTOR_ID_INDEX>(scene_tuple);
	scene.ID = ID;
	if (scene.onAttachedFunction)
		scene.onAttachedFunction(scene);
	return scene;
}
bool Window::removeScene(const Scene& scene)
{
	auto iter = scenes.find_id(scene.ID);
	if (iter == scenes.end())
	{
		return false;
	}
	scenes.erase(iter);
	return true;
}
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
void Window::updateDeltaTime(NANO_TIMEPOINT now, bool updateLastFrameDeltaTime)
{
	if (updateLastFrameDeltaTime)
	{
		auto duration = now - lastFrameTime;
		lastFrameDeltaTime = duration.count() / 1'000'000'000.0L;
	}
	lastFrameTime = now;
};
void Window::resize(glm::vec2 newSize)
{
	if (windowWidth != newSize.x)
		windowWidth = newSize.x;
	if (windowHeight != newSize.y)
		windowHeight = newSize.y;
	auto scenesSize = scenes.size();
	auto scenesData = scenes.data();
	for (size_t index = 0; index < scenesSize; ++index)
		scenesData[index].resize(newSize);
	callResizeHandler(newSize);
};
void Window::registerOnEntityAddedFunction(const OnEntityAddedFunction& function)
{
	onEntityAdded = function;
	return;
}

void zg::computeNormals(zg::FRONTFACE frontFace, const std::vector<uint32_t>& indices,
												const std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals)
{
	const float FACE_NORMAL_EPSILON_SQ = 1e-12f * 1e-12f;
	const float EDGE_LEN_EPSILON_SQ = 1e-10f * 1e-10f;
	const float VEC_EPSILON_SQ = 1e-10f * 1e-10f;
	// Prevent division by zero or extremely large weights from tiny edge products
	const float MIN_DENOMINATOR = EDGE_LEN_EPSILON_SQ * EDGE_LEN_EPSILON_SQ;


	const size_t nbVertices = vertices.size();
	const size_t nbIndices = indices.size();

	if (nbVertices == 0 || nbIndices == 0)
	{
		normals.clear();
		return;
	}
	if (nbIndices % 3 != 0)
	{
		normals.clear();
		return;
	}

	if (normals.size() != nbVertices)
	{
		normals.resize(nbVertices);
	}
	std::fill(normals.begin(), normals.end(), glm::vec3(0.0f));


	for (size_t i = 0; i < nbIndices; i += 3)
	{
		uint32_t i1 = indices[i + 0];
		uint32_t i2 = indices[i + 1];
		uint32_t i3 = indices[i + 2];

		if (i1 >= nbVertices || i2 >= nbVertices || i3 >= nbVertices)
		{
			continue;
		}

		const glm::vec3& p1 = vertices[i1];
		const glm::vec3& p2 = vertices[i2];
		const glm::vec3& p3 = vertices[i3];

		glm::vec3 fn_unnormalized;
		if (frontFace == zg::COUNTERCLOCKWISE)
		{
			fn_unnormalized = glm::cross(p2 - p1, p3 - p1);
		}
		else
		{
			fn_unnormalized = glm::cross(p3 - p1, p2 - p1);
		}

		float face_len_sq = glm::dot(fn_unnormalized, fn_unnormalized);
		if (face_len_sq < FACE_NORMAL_EPSILON_SQ)
		{
			continue; // Skip degenerate face
		}

		// Edges for vertex 1
		glm::vec3 e12 = p2 - p1;
		glm::vec3 e13 = p3 - p1;
		float len12_sq = glm::dot(e12, e12);
		float len13_sq = glm::dot(e13, e13);

		if (len12_sq > EDGE_LEN_EPSILON_SQ && len13_sq > EDGE_LEN_EPSILON_SQ)
		{
			float denominator = len12_sq * len13_sq;
			float weight = 1.0f / glm::max(MIN_DENOMINATOR, denominator);
			normals[i1] += fn_unnormalized * weight;
		}

		// Edges for vertex 2
		glm::vec3 e21 = p1 - p2; // -e12
		glm::vec3 e23 = p3 - p2;
		float len21_sq = len12_sq; // Reuse squared length
		float len23_sq = glm::dot(e23, e23);

		if (len21_sq > EDGE_LEN_EPSILON_SQ && len23_sq > EDGE_LEN_EPSILON_SQ)
		{
			float denominator = len21_sq * len23_sq;
			float weight = 1.0f / glm::max(MIN_DENOMINATOR, denominator);
			normals[i2] += fn_unnormalized * weight;
		}

		// Edges for vertex 3
		glm::vec3 e31 = p1 - p3; // -e13
		glm::vec3 e32 = p2 - p3; // -e23
		float len31_sq = len13_sq; // Reuse squared length
		float len32_sq = len23_sq; // Reuse squared length

		if (len31_sq > EDGE_LEN_EPSILON_SQ && len32_sq > EDGE_LEN_EPSILON_SQ)
		{
			float denominator = len31_sq * len32_sq;
			float weight = 1.0f / glm::max(MIN_DENOMINATOR, denominator);
			normals[i3] += fn_unnormalized * weight;
		}
	}

	// Normalize final normals
	for (size_t i = 0; i < nbVertices; ++i)
	{
		float normal_len_sq = glm::dot(normals[i], normals[i]);

		if (normal_len_sq > VEC_EPSILON_SQ)
		{
			normals[i] = normals[i] / std::sqrt(normal_len_sq);
		}
		else
		{
			// Fallback for zero-length normals
			const glm::vec3& pos = vertices[i];
			float pos_len_sq = glm::dot(pos, pos);
			if (pos_len_sq > VEC_EPSILON_SQ)
			{
				normals[i] = pos / std::sqrt(pos_len_sq);
			}
			else
			{
				normals[i] = glm::vec3(0.0f, 0.0f, 1.0f); // Default Z-up
			}
		}
	}
}

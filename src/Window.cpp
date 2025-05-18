#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <zg/textures/Texture.hpp>
#include <zg/Registry.hpp>
#if defined(MACOS)
#include <zg/windows/MacOSWindow.hpp>
#elif defined(__linux__)
#include <zg/windows/WaylandWindow.hpp>
#include <zg/windows/X11Window.hpp>
#include <zg/windows/XCBWindow.hpp>
#elif defined(_WIN32)
#include <zg/windows/WIN32Window.hpp>
#endif
using namespace zg;
	#include <zg/windows/WIN32Window.hpp>
#ifdef _WIN32
extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
Window::Window(const WindowCreateInfo& info) :
	ID(info.ID),
	INDEX(info.INDEX),
	INDEX_STACK(info.INDEX_STACK),
	title(info.title), childWindows([](auto& childWindow) { return childWindow.title; }), windowWidth(info.windowWidth),
	windowHeight(info.windowHeight), windowX(info.windowX), windowY(info.windowY),
	scenes([](auto& scene) { return scene.name; }), deltaTime(1.0 / info.framerate), borderless(info.borderless),
	framerate(info.framerate), vsync(info.vsync), frameduration(NANOSECONDS_DURATION(deltaTime * NANOSECONDS::den)),
	framebudget(frameduration), systemFonts(*this), postProcessingPipeline(INDEX_STACK)
{
	setViewport();
	ZoneScoped;
	memset(windowKeys, 0, 256 * sizeof(bool));
	memset(windowButtons, 0, 7 * sizeof(bool));
	// if (info.isChildWindow)
	// {
	// 	NDCFramebufferPlane = info.NDCFramebufferPlane;
	// 	framebufferTexture = std::make_shared<textures::Texture>(
	// 		info.parentWindowPointer->iRenderer, glm::ivec4(info.windowWidth, info.windowHeight, 1, 0), (void*)0);
	// 	framebufferDepthTexture = std::make_shared<textures::Texture>(
	// 		info.parentWindowPointer->iRenderer, glm::ivec4(info.windowWidth, info.windowHeight, 1, 0), (void*)0);
	// 	framebuffer = std::make_shared<textures::Framebuffer>(
	// 		info.parentWindowPointer->iRenderer,
	// 		std::vector<textures::Framebuffer::TextureAttachmentPair>(
	// 			{{framebufferTexture.get(),
	// 				textures::Framebuffer::AttachmentType::
	// 					Color} /*, {framebufferDepthTexture.get(), textures::Framebuffer::AttachmentType::Depth}*/}));
	// 	framebufferPlane->addToBVH = false;
	// }
}
Window::Window(const Window& other) :
	ID(other.ID),
	INDEX(other.INDEX),
	INDEX_STACK(other.INDEX_STACK),
		title(other.title), childWindows(other.childWindows), windowWidth(other.windowWidth),
		windowHeight(other.windowHeight), windowX(other.windowX), windowY(other.windowY), scenes(other.scenes),
		deltaTime(1.0 / other.framerate), borderless(other.borderless), framerate(other.framerate), vsync(other.vsync),
		frameduration(NANOSECONDS_DURATION(deltaTime * NANOSECONDS::den)), framebudget(frameduration), systemFonts(*this),
		postProcessingPipeline(INDEX_STACK),
		mainColorTexture(other.mainColorTexture),// mainDepthTexture(other.mainDepthTexture),
		mainFramebuffer(other.mainFramebuffer)
{
	setViewport();
	ZoneScoped;
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
}
Window& Window::operator=(const Window& other)
{
	ZoneScoped;
	ID = other.ID;
	INDEX = other.INDEX;
	INDEX_STACK = other.INDEX_STACK;
	iPlatformWindow = other.iPlatformWindow;
	iRenderer = other.iRenderer;
	windowWidth = other.windowWidth;
	windowHeight = other.windowHeight;
	setViewport();
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
	newMouseCoords = other.newMouseCoords;
	mod = other.mod;
	isChildWindow = other.isChildWindow;
	parentWindow = other.parentWindow;
	parentScene = other.parentScene;
	childWindows = other.childWindows;
	NDCFramebufferPlane = other.NDCFramebufferPlane;
	framebufferTexture = other.framebufferTexture;
	framebufferDepthTexture = other.framebufferDepthTexture;
	framebuffer = other.framebuffer;
	framebufferPlane = other.framebufferPlane;
	oldXY = other.oldXY;
	vsync = other.vsync;
	frameduration = other.frameduration;
	mainColorTexture = other.mainColorTexture;
	// mainDepthTexture = other.mainDepthTexture;
	mainFramebuffer = other.mainFramebuffer;
	return *this;
}
void Window::run()
{
	ZoneScoped;
// #if defined(_WIN32) || defined(__linux__)
// 	windowThread = std::make_shared<std::thread>(&Window::startWindow, this);
// 	windowThread->join();
// #elif defined(MACOS)
	startWindow();
// #endif
}
void Window::update()
{
	ZoneScopedN("Window::update");
	auto componentsData = m_components.data();
	auto componentsSize = m_components.size();
	for (size_t index = 0; index < componentsSize; ++index)
	{
		ZoneScopedN("component:onUpdate");
		componentsData[index].onUpdate();
	}
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
	{
		ZoneScopedN("scene:update");
		scenesData[index].update();
	}
}
void Window::preRender()
{
	ZoneScopedN("Window::preRender");
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
	{
		ZoneScopedN("scene:preRender");
		scenesData[index].preRender();
	}
	if (!isChildWindow)
		return;
	runRunnables();
	updateKeyboard();
	updateMouse();
	auto& framebufferRef = *framebuffer;
	framebufferRef.bind();
}
void Window::render()
{
	ZoneScopedN("Window::render");
	std::lock_guard lock(renderMutex);
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
	{
		ZoneScopedN("scene:render");
		scenesData[index].render();
	}
};
void Window::postRender()
{
	ZoneScopedN("Window::postRender");
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
	{
		ZoneScopedN("scene:postRender");
		scenesData[index].postRender();
	}
	if (!isChildWindow)
		return;
	framebuffer->unbind();
}
void Window::startWindow()
{
	ZoneScopedN("Window::startWindow");
	iPlatformWindow = createPlatformWindow();
	auto& iPlatformWindowRef = *iPlatformWindow;
	iRenderer = createRenderer();
	auto& iRendererRef = *iRenderer;
	iPlatformWindowRef.init(*this);
	iRendererRef.createContext(&iPlatformWindowRef);
	iRendererRef.init();
	iPlatformWindowRef.postInit();
	fullscreenQuad = std::make_unique<FullscreenQuad>(INDEX_STACK, zg::shaders::RuntimeConstants({"ColorTexture"}));
	mainColorTexture = std::make_shared<textures::Texture>(iRenderer, glm::ivec4(windowWidth, windowHeight, 1, 0), (const void*)0, textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte, textures::Texture::FilterType::Linear, true, textures::Texture::Multisampling::x1);
	// mainDepthTexture = std::make_shared<textures::Texture>(iRenderer, glm::ivec4(windowWidth, windowHeight, 1, 0), (const void*)0, textures::Texture::Format::Depth, textures::Texture::Type::Float, textures::Texture::FilterType::Linear, true, textures::Texture::Multisampling::x1);
	mainFramebuffer = std::make_shared<textures::Framebuffer>(iRenderer, std::vector<textures::Framebuffer::TextureAttachmentPair>{
		{mainColorTexture, textures::Framebuffer::AttachmentType::Color}//,
		// {mainDepthTexture, textures::Framebuffer::AttachmentType::Depth}
	});
	postProcessingPipeline.textureRegistry.registerOutput((std::numeric_limits<float>::lowest)(), "ColorTexture", mainColorTexture);
	// postProcessingPipeline.textureRegistry.registerOutput((std::numeric_limits<float>::lowest)(), "DepthTexture", mainDepthTexture);
	runRunnables();
	iPlatformWindowRef.disableKeyAutoRepeat();
	std::vector<std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>>> ppOutputs;
	while (true)
	{
		ZoneScopedN("window:mainLoop");
		{
			ZoneScopedN("mainLoop:budgetBegin");
			auto _now = framebudget.begin();
			updateDeltaTime(_now, false);
		}
		{
			{
				ZoneScopedN("mainLoop:update");
				if (!iPlatformWindowRef.pollMessages())
				{
					framebudget.sleep();
					break;
				}
				iRendererRef.preBeginRenderPass();
				runRunnables();
				updateKeyboard();
				updateMouse();
				update();
			}
			// auto childWindowsSize = childWindows.size();
			// auto childWindowsData = childWindows.data();
			// for (size_t index = 0; index < childWindowsSize; ++index)
			// {
			// 	auto& childWindow = childWindowsData[index];
			// 	if (childWindow.minimized)
			// 		continue;
			// 	childWindow.render();
			// 	framebudget.tick();
			// }
			// iRendererRef.beginMainFramebuffer();
			// for (size_t index = 0; index < childWindowsSize; ++index)
			// {
			// 	auto& childWindow = childWindowsData[index];
			// 	if (childWindow.minimized)
			// 		continue;
			// 	framebudget.tick();
			// 	childWindow.framebufferPlane->render();
			// }
			// iRendererRef.postMainFramebuffer();
			{
				ZoneScopedN("mainLoop:renderPass");
				preRender();
				render();
				ppOutputs.resize(scenes.size());
				auto index = 0;
				auto scenesData = scenes.data();
				auto scenesSize = scenes.size();
				for (size_t index = 0; index < scenesSize; ++index)
				{
					ZoneScopedN("p3:pp");
					auto& scene = scenesData[index];
					ppOutputs[index++] = scene.postProcessingPipeline.postProcess();
				}
				index = 0;
				mainFramebuffer->bind();
				for (auto& sceneID : sortedScenes)
				{
					ZoneScopedN("p3/scene:render");
					auto& scene = Registry::getScene(sceneID);
					scene.fsq->render(ppOutputs[index++]);
				}
				mainFramebuffer->unbind();
				{
					ZoneScopedN("mainLoop: p3:final");
					auto finalInputs = postProcessingPipeline.postProcess();
					{
						ZoneScopedN("mainLoop: final fsq render");
						iRendererRef.beginRenderPass();
						fullscreenQuad->render(finalInputs);
					}
					{
						ZoneScopedN("mainLoop:postRender");
						iRendererRef.postRenderPass();
						postRender();
					}
				}
			}
			{
				ZoneScopedN("mainLoop:budgetEnd");
				auto _now = framebudget.end();
				updateDeltaTime(_now, true);
			}
		}
		FrameMark;
		{
			ZoneScopedN("mainLoop:swapBuffers");
			callPreSwapbuffersOnceoff();
			iRendererRef.swapBuffers();
			framebudget.sleep();
		}
	}
_exit:
	for (auto& pair : shutdownHandlers)
	{
		ZoneScopedN("shutdownHandler");
		pair.second(*this);
	}
	iPlatformWindowRef.enableKeyAutoRepeat();
	audioEngine.stop();
	audioEngine.clearPipeline();
	childWindows.clear();
	{
		auto scenesSize = scenes.size();
		auto scenesData = scenes.data();
		for (size_t i = 0; i < scenesSize; ++i)
		{
			ZoneScoped;
			scenesData[i].detachAllComponents();
		}
	}
	scenes.clear();
	mainColorTexture.reset();
	// mainDepthTexture.reset();
	mainFramebuffer.reset();
	fullscreenQuad.reset();
	ppOutputs.clear();
	postProcessingPipeline.cleanup();
	detachAllComponents();
    delete iRenderer->shaderContext;
    iRendererRef.destroy();
	delete iRenderer;
	iPlatformWindowRef.destroy();
	delete iPlatformWindow;
	zg::Entity::cleanupSerialize();
	if (Registry::idWindows)
	{
		auto& idWindowsRef = *Registry::idWindows;
		if (!idWindowsRef.size())
			return;
		auto idIter = idWindowsRef.find(ID);
		if (idIter != idWindowsRef.end())
		{
			idWindowsRef.erase(idIter);
		}
	}
}
void Window::updateKeyboard()
{
	ZoneScoped;
	for (unsigned int i = 0; i < 256; ++i)
	{
		ZoneScoped;
		auto& pressed = windowKeys[i];
		if (keys[i] != pressed)
		{
			ZoneScoped;
			callKeyPressHandler(i, pressed);
			callAnyKeyPressHandler(i, pressed);
		}
		if (pressed)
		{
			ZoneScoped;
			callKeyUpdateHandler(i);
		}
	}
}
void Window::updateMouse()
{
	ZoneScoped;
	for (unsigned int i = MinMouseButtonIndex; i < MaxMouseButtonIndex; ++i)
	{
		ZoneScoped;
	_checkPressed:
		auto& pressed = windowButtons[i];
		if (buttons[i] != pressed)
		{
			ZoneScoped;
			callMousePressHandler(i, pressed);
			if ((i == 3 || i == 4) && pressed)
			{
				ZoneScoped;
				windowButtons[i] = false;
				goto _checkPressed;
			}
		}
	}
	if (mouseMoved)
	{
		ZoneScoped;
		callMouseMoveHandler(newMouseCoords);
		mouseMoved = false;
	}
}
void Window::close()
{
	ZoneScoped;
	if (isChildWindow)
	{
		ZoneScoped;
		return;
	}
	iPlatformWindow->close();
}
void Window::minimize()
{
	ZoneScoped;
	minimized = true;
	maximized = false;
	if (isChildWindow)
	{
		ZoneScoped;
		return;
	}
	iPlatformWindow->minimize();
	buttons.clear();
	for (unsigned i = 0; i <= MaxMouseButtonIndex; ++i)
	{
		windowButtons[i] = false;
	}
}
void Window::maximize()
{
	ZoneScoped;
	minimized = false;
	if (maximized)
	{
		ZoneScoped;
		maximized = false;
		iPlatformWindow->restore();
		setXY(oldXY.x, oldXY.y);
	}
	else
	{
		ZoneScoped;
		maximized = true;
		iPlatformWindow->maximize();
		oldXY.x = windowX;
		oldXY.y = windowY;
		setXY(0, 0);
	}
}
void Window::restore()
{
	ZoneScoped;
	minimized = false;
	maximized = false;
	if (isChildWindow)
	{
		ZoneScoped;
		return;
	}
	iPlatformWindow->restore();
	setXY(oldXY.x, oldXY.y);
}
void Window::warpPointer(glm::vec2 coords)
{
	ZoneScoped;
	iPlatformWindow->warpPointer(coords);
	justWarpedPointer = true;
}
void Window::setXY(float x, float y)
{
	ZoneScoped;
	windowX = x;
	windowY = y;
	if (isChildWindow)
	{
		ZoneScoped;
		return;
	}
	iPlatformWindow->setXY();
}
void Window::setWidthHeight(float width, float height)
{
	ZoneScoped;
	windowWidth = width;
	windowHeight = height;
	setViewport();
	if (isChildWindow)
	{
		ZoneScoped;
		return;
	}
	iPlatformWindow->setWidthHeight();
}
void Window::setViewport()
{
	viewport = {0, 0, windowWidth, windowHeight};
}
void Window::mouseCapture(bool capture)
{
	ZoneScoped;
	iPlatformWindow->mouseCapture(capture);
}
zg::Window& Window::createChildWindow(const WindowCreateInfo& info)
{
	ZoneScoped;
	auto usingInfo{info};
	usingInfo.isChildWindow = true;
	return *std::get<KEY_ID_VECTOR_VALUE_INDEX>(childWindows.emplace_back(usingInfo));
}

// Keyboard
UniqueIdentifier Window::addKeyPressHandler(Key key, const KeyPressHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyPressHandlers[key];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void Window::removeKeyPressHandler(Key key, UniqueIdentifier& id)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyPressHandlers[key];
	auto handlerIter = handlersPair.second.find(id);
	if (handlerIter == handlersPair.second.end())
	{
		ZoneScoped;
		return;
	}
	handlersPair.second.erase(handlerIter);
	id = 0;
};
UniqueIdentifier Window::addKeyUpdateHandler(Key key, const KeyUpdateHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyUpdateHandlers[key];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void Window::removeKeyUpdateHandler(Key key, UniqueIdentifier& id)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto handlersIter = keyUpdateHandlers.find(key);
	if (handlersIter == keyUpdateHandlers.end())
	{
		ZoneScoped;
		return;
	}
	auto& handlers = handlersIter->second.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		ZoneScoped;
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void Window::callKeyPressHandler(Key key, int pressed)
{
	ZoneScoped;
	keys[key] = pressed;
	{
		std::vector<KeyPressHandler> handlersCopy;
		{
			// std::lock_guard lock(handlersMutex);
			auto handlersIter = keyPressHandlers.find(key);
			if (handlersIter == keyPressHandlers.end())
			{
				ZoneScoped;
				return;
			}
			auto& handlersMap = handlersIter->second.second;
			for (const auto& pair : handlersMap)
			{
				ZoneScoped;
				handlersCopy.push_back(pair.second);
			}
		}
		for (auto& handler : handlersCopy)
		{
			ZoneScoped;
			handler(!!pressed);
		}
	}
};
void Window::callKeyUpdateHandler(Key key)
{
	ZoneScoped;
	std::vector<KeyUpdateHandler> handlersCopy;
	{
		// std::lock_guard lock(handlersMutex);
		auto handlersIter = keyUpdateHandlers.find(key);
		if (handlersIter == keyUpdateHandlers.end())
		{
			ZoneScoped;
			return;
		}
		auto& handlersMap = handlersIter->second.second;
		for (const auto& pair : handlersMap)
		{
			ZoneScoped;
			handlersCopy.push_back(pair.second);
		}
	}
	for (auto& handler : handlersCopy)
	{
		ZoneScoped;
		handler();
	}
};
UniqueIdentifier Window::addAnyKeyPressHandler(const AnyKeyPressHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++anyKeyPressHandlers.first;
	anyKeyPressHandlers.second[id] = callback;
	return id;
};
void Window::removeAnyKeyPressHandler(UniqueIdentifier& id)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlers = anyKeyPressHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		ZoneScoped;
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void Window::callAnyKeyPressHandler(Key key, bool pressed)
{
	ZoneScoped;
	std::vector<AnyKeyPressHandler> handlersCopy;
	{
		ZoneScoped;
		// std::lock_guard lock(handlersMutex);
		auto& handlersMap = anyKeyPressHandlers.second;
		for (const auto& pair : handlersMap)
		{
			ZoneScoped;
			handlersCopy.push_back(pair.second);
		}
	}
	for (auto& handler : handlersCopy)
	{
		ZoneScoped;
		handler(key, pressed);
	}
}
void Window::handleKey(Key key, int32_t mod, bool pressed)
{
	ZoneScoped;
	auto& window = *dynamic_cast<Window*>(this);
	bool hadChildFocus = false;
	auto childWindowsSize = childWindows.size();
	auto childWindowsData = childWindows.data();
	for (size_t index = 0; index < childWindowsSize; ++index)
	{
		ZoneScoped;
		auto& childWindow = childWindowsData[index];
		if (childWindow.minimized)
		{
			ZoneScoped;
			continue;
		}
		if (!childWindow.focused)
		{
			ZoneScoped;
			continue;
		}
		childWindow.mod = mod;
		childWindow.windowKeys[key] = pressed;
		hadChildFocus = true;
		break;
	}
	if (!hadChildFocus)
	{
		ZoneScoped;
		window.mod = mod;
		window.windowKeys[key] = pressed;
	}
}
// Mouse
UniqueIdentifier Window::addMousePressHandler(Button button, const MousePressHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = mousePressHandlers[button];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void Window::removeMousePressHandler(Button button, UniqueIdentifier& id)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = mousePressHandlers[button];
	auto handlerIter = handlersPair.second.find(id);
	if (handlerIter == handlersPair.second.end())
	{
		ZoneScoped;
		return;
	}
	handlersPair.second.erase(handlerIter);
	id = 0;
};
UniqueIdentifier Window::addMouseMoveHandler(const MouseMoveHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++mouseMoveHandlers.first;
	mouseMoveHandlers.second[id] = callback;
	return id;
};
void Window::removeMouseMoveHandler(UniqueIdentifier& id)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlers = mouseMoveHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		ZoneScoped;
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void Window::callMousePressHandler(Button button, bool pressed)
{
	ZoneScoped;
	buttons[button] = pressed;
	{
		ZoneScoped;
		std::vector<MousePressHandler> handlersCopy;
		{
			ZoneScoped;
			// std::lock_guard lock(handlersMutex);
			auto handlersIter = mousePressHandlers.find(button);
			if (handlersIter == mousePressHandlers.end())
			{
				ZoneScoped;
				return;
			}
			auto& handlersMap = handlersIter->second.second;
			for (const auto& pair : handlersMap)
			{
				ZoneScoped;
				handlersCopy.push_back(pair.second);
			}
		}
		for (auto& handler : handlersCopy)
		{
			ZoneScoped;
			handler(!!pressed);
		}
	}
}
void Window::callMouseMoveHandler(glm::vec2 coords)
{
	ZoneScoped;
	if (coords == mouseCoords)
	{
		ZoneScoped;
		return;
	}
	mouseCoords = coords;
	std::vector<MouseMoveHandler> handlersCopy;
	{
		ZoneScoped;
		// std::lock_guard lock(handlersMutex);
		auto& handlersMap = mouseMoveHandlers.second;
		for (const auto& pair : handlersMap)
		{
			ZoneScoped;
			handlersCopy.push_back(pair.second);
		}
	}
	for (auto& handler : handlersCopy)
	{
		ZoneScoped;
		handler(coords);
	}
}
void Window::handleMouseMove(uint32_t x, uint32_t y)
{
	ZoneScoped;
	auto& window = *dynamic_cast<Window*>(this);
	bool hadChildFocus = false;
	auto childWindowsSize = childWindows.size();
	auto childWindowsData = childWindows.data();
	for (size_t index = 0; index < childWindowsSize; ++index)
	{
		ZoneScoped;
		auto& childWindow = childWindowsData[index];
		if (childWindow.minimized)
		{
			ZoneScoped;
			continue;
		}
		if (!childWindow.focused)
		{
			ZoneScoped;
			continue;
		}
		auto childX = x - childWindow.windowX;
		auto childY = childWindow.windowHeight - (window.windowHeight - y - childWindow.windowY);
		childWindow.newMouseCoords.x = childX, childWindow.newMouseCoords.y = childY;
		childWindow.mouseMoved = true;
		hadChildFocus = true;
		break;
	}
	if (!hadChildFocus)
	{
		ZoneScoped;
		window.newMouseCoords.y = y, window.newMouseCoords.x = x;
		window.mouseMoved = true;
	}
}
void Window::handleMousePress(Button button, bool pressed)
{
	ZoneScoped;
	auto& window = *dynamic_cast<Window*>(this);
	bool hadChildFocus = false;
	auto childWindowsSize = childWindows.size();
	auto childWindowsData = childWindows.data();
	for (size_t index = 0; index < childWindowsSize; ++index)
	{
		ZoneScoped;
		auto& childWindow = childWindowsData[index];
		if (childWindow.minimized)
		{
			ZoneScoped;
			continue;
		}
		if (!childWindow.focused)
		{
			ZoneScoped;
			continue;
		}
		childWindow.windowButtons[button] = pressed;
		hadChildFocus = true;
		break;
	}
	if (!hadChildFocus)
	{
		ZoneScoped;
		window.windowButtons[button] = pressed;
	}
}
// resize
UniqueIdentifier Window::addResizeHandler(const ViewResizeHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++viewResizeHandlers.first;
	viewResizeHandlers.second[id] = callback;
	return id;
};
void Window::removeResizeHandler(UniqueIdentifier& id)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlers = viewResizeHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		ZoneScoped;
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void Window::callResizeHandler(glm::vec2 newSize)
{
	ZoneScoped;
	std::vector<ViewResizeHandler> handlersCopy;
	{
		ZoneScoped;
		// std::lock_guard lock(handlersMutex);
		auto& handlersMap = viewResizeHandlers.second;
		for (const auto& pair : handlersMap)
		{
			ZoneScoped;
			handlersCopy.push_back(pair.second);
		}
	}
	for (auto& handler : handlersCopy)
	{
		ZoneScoped;
		handler(newSize);
	}
};
// focus
UniqueIdentifier Window::addFocusHandler(const FocusHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++focusHandlers.first;
	focusHandlers.second[id] = callback;
	return id;
}
void Window::removeFocusHandler(UniqueIdentifier& id)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlers = focusHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		ZoneScoped;
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
}
void Window::callFocusHandler(bool focused)
{
	ZoneScoped;
	if (this->focused == focused)
	{
		ZoneScoped;
		return;
	}
	std::vector<FocusHandler> handlersCopy;
	{
		ZoneScoped;
		// std::lock_guard lock(handlersMutex);
		auto& handlersMap = focusHandlers.second;
		for (const auto& pair : handlersMap)
		{
			ZoneScoped;
			handlersCopy.push_back(pair.second);
		}
	}
	this->focused = focused;
	for (auto& handler : handlersCopy)
	{
		ZoneScoped;
		handler(focused);
	}
}
// onceoffs
void Window::addPreSwapbuffersOnceoff(const PreSwapbuffersOnceoff& onceoff)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	preSwapbuffersOnceoffs.push(onceoff);
}
void Window::callPreSwapbuffersOnceoff()
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	if (preSwapbuffersOnceoffs.empty())
	{
		ZoneScoped;
		return;
	}
	auto onceoff = preSwapbuffersOnceoffs.front();
	preSwapbuffersOnceoffs.pop();
	onceoff();
}
size_t Window::addShutdownHandler(const ShutdownHandler& handler)
{
	ZoneScoped;
	auto ID = GlobalUID::GetNew();
	shutdownHandlers[ID] = handler;
	return ID;
}
bool Window::removeShutdownHandler(size_t& ID)
{
	ZoneScoped;
	auto iter = shutdownHandlers.find(ID);
	if (iter == shutdownHandlers.end())
	{
		ZoneScoped;
		return false;
	}
	shutdownHandlers.erase(iter);
	ID = 0;
	return true;
}
KeyIDVector<std::string, Scene>::EmplaceBackTuple Window::addScene(const SceneCreateInfo& info)
{
	ZoneScoped;
	auto usingInfo{info};
	auto transaction = scenes.startTransaction();
	usingInfo.INDEX_STACK = {INDEX_STACK.begin(), INDEX_STACK.end()};
	usingInfo.INDEX_STACK.push_back(transaction.index);
	usingInfo.ID = transaction.id;
	usingInfo.INDEX = transaction.index;
	auto& scene = scenes.commitTransaction(transaction, usingInfo);
	sceneZ = (scene.z = (sceneZ + 0.1f));
	if (scene.onAttachedFunction)
	{
		ZoneScoped;
		scene.onAttachedFunction(scene);
	}
	sortedScenes.push_back(scene.ID);
	sortScenes();
	(*Registry::idScenes)[scene.ID] = scene.INDEX_STACK;
	return {transaction.key, transaction.id, transaction.index, &scene};
}
bool Window::removeScene(size_t ID)
{
	ZoneScoped;
	auto iter = scenes.find_id(ID);
	if (iter == scenes.end())
	{
		ZoneScoped;
		return false;
	}
	auto& scene = *iter;
	if (scene.onDetachedFunction)
	{
		ZoneScoped;
		scene.onDetachedFunction(scene);
	}
	sceneZ -= 0.1f;
	auto sortedSceneIter = std::find(sortedScenes.begin(), sortedScenes.end(), scene.ID);
	sortedScenes.erase(sortedSceneIter);
	scenes.erase(iter);
	sortScenes();
	auto& idScenesRef = *Registry::idScenes;
	auto idIter = idScenesRef.find(ID);
	if (idIter != idScenesRef.end())
	{
		ZoneScoped;
		idScenesRef.erase(idIter);
	}
	return true;
}
void Window::sortScenes()
{
	std::sort(sortedScenes.begin(), sortedScenes.end(), [](auto& a_ID, auto& b_ID){
		auto& a_s = Registry::getScene(a_ID);
		auto& b_s = Registry::getScene(b_ID);
		return a_s.z > b_s.z;
	});
}
void Window::runOnThread(const Runnable& runnable)
{
	ZoneScoped;
	// std::lock_guard lock(runnablesMutex);
	runnables.push(runnable);
};
void Window::runRunnables()
{
	ZoneScoped;
	std::queue<Runnable> runnablesCopy;
	{
		ZoneScoped;
		// std::lock_guard lock(runnablesMutex);
		runnablesCopy = runnables;
		while (!runnables.empty())
		{
			ZoneScoped;
			runnables.pop();
		}
	}
	while (!runnablesCopy.empty())
	{
		ZoneScoped;
		auto runnable = runnablesCopy.front();
		runnablesCopy.pop();
		runnable(dynamic_cast<Window&>(*this));
	}
};
void Window::updateDeltaTime(NANO_TIMEPOINT now, bool updateLastFrameDeltaTime)
{
	ZoneScoped;
	if (updateLastFrameDeltaTime)
	{
		ZoneScoped;
		auto duration = now - lastFrameTime;
		lastFrameDeltaTime = duration.count() / 1'000'000'000.0L;
	}
	lastFrameTime = now;
};
void Window::resize(glm::vec2 newSize)
{
	ZoneScoped;
	if (windowWidth != newSize.x)
		windowWidth = newSize.x;
	if (windowHeight != newSize.y)
		windowHeight = newSize.y;
	auto scenesSize = scenes.size();
	auto scenesData = scenes.data();
	for (size_t index = 0; index < scenesSize; ++index)
	{
		ZoneScoped;
		scenesData[index].resize(newSize);
	}
	callResizeHandler(newSize);
};
void Window::registerOnEntityAddedFunction(const OnEntityAddedFunction& function)
{
	ZoneScoped;
	onEntityAdded = function;
	return;
}
uint32_t Window::getScreenRefreshRate(uint32_t screenNum)
{
	ZoneScoped;
	auto modes =
#if defined(_WIN32)
	WIN32Window::getCurrentScreenModes();
#elif defined(__linux__)
	XCBWindow::getCurrentScreenModes();
#elif defined(MACOS)
	MacOSWindow::getCurrentScreenModes();
#endif
	return modes.size() >= screenNum ? modes[screenNum - 1].refreshRate : 60;
}

void zg::computeNormals(zg::FRONTFACE frontFace, const std::vector<uint32_t>& indices,
												const std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals)
{
	ZoneScoped;
	const float FACE_NORMAL_EPSILON_SQ = 1e-12f * 1e-12f;
	const float EDGE_LEN_EPSILON_SQ = 1e-10f * 1e-10f;
	const float VEC_EPSILON_SQ = 1e-10f * 1e-10f;
	// Prevent division by zero or extremely large weights from tiny edge products
	const float MIN_DENOMINATOR = EDGE_LEN_EPSILON_SQ * EDGE_LEN_EPSILON_SQ;


	const size_t nbVertices = vertices.size();
	const size_t nbIndices = indices.size();

	if (nbVertices == 0 || nbIndices == 0)
	{
		ZoneScoped;
		normals.clear();
		return;
	}
	if (nbIndices % 3 != 0)
	{
		ZoneScoped;
		normals.clear();
		return;
	}

	if (normals.size() != nbVertices)
	{
		ZoneScoped;
		normals.resize(nbVertices);
	}
	std::fill(normals.begin(), normals.end(), glm::vec3(0.0f));


	for (size_t i = 0; i < nbIndices; i += 3)
	{
		ZoneScoped;
		uint32_t i1 = indices[i + 0];
		uint32_t i2 = indices[i + 1];
		uint32_t i3 = indices[i + 2];

		if (i1 >= nbVertices || i2 >= nbVertices || i3 >= nbVertices)
		{
			ZoneScoped;
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
		ZoneScoped;
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

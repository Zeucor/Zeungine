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
zg::Window::Window(const WindowCreateInfo& info) :
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
zg::Window::Window(const zg::Window& other) :
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
zg::Window& zg::Window::operator=(const zg::Window& other)
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
void zg::Window::run()
{
	ZoneScoped;
// #if defined(_WIN32) || defined(__linux__)
// 	windowThread = std::make_shared<std::thread>(&zg::Window::startWindow, this);
// 	windowThread->join();
// #elif defined(MACOS)
	startWindow();
// #endif
}
void zg::Window::update()
{
	ZoneScopedN("zg::Window::update");
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
void zg::Window::preRender()
{
	ZoneScopedN("zg::Window::preRender");
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
void zg::Window::render()
{
	ZoneScopedN("zg::Window::render");
	std::lock_guard lock(renderMutex);
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
	{
		ZoneScopedN("scene:render");
		scenesData[index].render();
	}
};
void zg::Window::postRender()
{
	ZoneScopedN("zg::Window::postRender");
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
void zg::Window::startWindow()
{
	ZoneScopedN("zg::Window::startWindow");
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
void zg::Window::updateKeyboard()
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
void zg::Window::updateMouse()
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
void zg::Window::close()
{
	ZoneScoped;
	if (isChildWindow)
	{
		ZoneScoped;
		return;
	}
	iPlatformWindow->close();
}
void zg::Window::minimize()
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
void zg::Window::maximize()
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
void zg::Window::restore()
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
void zg::Window::warpPointer(glm::vec2 coords)
{
	ZoneScoped;
	iPlatformWindow->warpPointer(coords);
	justWarpedPointer = true;
}
void zg::Window::setXY(float x, float y)
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
void zg::Window::setWidthHeight(float width, float height)
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
void zg::Window::setViewport()
{
	viewport = {0, 0, windowWidth, windowHeight};
}
void zg::Window::mouseCapture(bool capture)
{
	ZoneScoped;
	iPlatformWindow->mouseCapture(capture);
}
zg::Window& zg::Window::createChildWindow(const WindowCreateInfo& info)
{
	ZoneScoped;
	auto usingInfo{info};
	usingInfo.isChildWindow = true;
	return *std::get<KEY_ID_VECTOR_VALUE_INDEX>(childWindows.emplace_back(usingInfo));
}

// Keyboard
UniqueIdentifier zg::Window::addKeyPressHandler(Key key, const KeyPressHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyPressHandlers[key];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void zg::Window::removeKeyPressHandler(Key key, UniqueIdentifier& id)
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
UniqueIdentifier zg::Window::addKeyUpdateHandler(Key key, const KeyUpdateHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyUpdateHandlers[key];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void zg::Window::removeKeyUpdateHandler(Key key, UniqueIdentifier& id)
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
void zg::Window::callKeyPressHandler(Key key, int pressed)
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
void zg::Window::callKeyUpdateHandler(Key key)
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
UniqueIdentifier zg::Window::addAnyKeyPressHandler(const AnyKeyPressHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++anyKeyPressHandlers.first;
	anyKeyPressHandlers.second[id] = callback;
	return id;
};
void zg::Window::removeAnyKeyPressHandler(UniqueIdentifier& id)
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
void zg::Window::callAnyKeyPressHandler(Key key, bool pressed)
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
void zg::Window::handleKey(Key key, int32_t mod, bool pressed)
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
UniqueIdentifier zg::Window::addMousePressHandler(Button button, const MousePressHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = mousePressHandlers[button];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void zg::Window::removeMousePressHandler(Button button, UniqueIdentifier& id)
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
UniqueIdentifier zg::Window::addMouseMoveHandler(const MouseMoveHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++mouseMoveHandlers.first;
	mouseMoveHandlers.second[id] = callback;
	return id;
};
void zg::Window::removeMouseMoveHandler(UniqueIdentifier& id)
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
void zg::Window::callMousePressHandler(Button button, bool pressed)
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
void zg::Window::callMouseMoveHandler(glm::vec2 coords)
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
void zg::Window::handleMouseMove(uint32_t x, uint32_t y)
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
void zg::Window::handleMousePress(Button button, bool pressed)
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
UniqueIdentifier zg::Window::addResizeHandler(const ViewResizeHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++viewResizeHandlers.first;
	viewResizeHandlers.second[id] = callback;
	return id;
};
void zg::Window::removeResizeHandler(UniqueIdentifier& id)
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
void zg::Window::callResizeHandler(glm::vec2 newSize)
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
UniqueIdentifier zg::Window::addFocusHandler(const FocusHandler& callback)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++focusHandlers.first;
	focusHandlers.second[id] = callback;
	return id;
}
void zg::Window::removeFocusHandler(UniqueIdentifier& id)
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
void zg::Window::callFocusHandler(bool focused)
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
void zg::Window::addPreSwapbuffersOnceoff(const PreSwapbuffersOnceoff& onceoff)
{
	ZoneScoped;
	// std::lock_guard lock(handlersMutex);
	preSwapbuffersOnceoffs.push(onceoff);
}
void zg::Window::callPreSwapbuffersOnceoff()
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
size_t zg::Window::addShutdownHandler(const ShutdownHandler& handler)
{
	ZoneScoped;
	auto ID = GlobalUID::GetNew();
	shutdownHandlers[ID] = handler;
	return ID;
}
bool zg::Window::removeShutdownHandler(size_t& ID)
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
KeyIDVector<std::string, Scene>::EmplaceBackTuple zg::Window::addScene(const SceneCreateInfo& info)
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
bool zg::Window::removeScene(size_t ID)
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
void zg::Window::sortScenes()
{
	std::sort(sortedScenes.begin(), sortedScenes.end(), [](auto& a_ID, auto& b_ID){
		auto& a_s = Registry::getScene(a_ID);
		auto& b_s = Registry::getScene(b_ID);
		return a_s.z > b_s.z;
	});
}
void zg::Window::runOnThread(const Runnable& runnable)
{
	ZoneScoped;
	// std::lock_guard lock(runnablesMutex);
	runnables.push(runnable);
};
void zg::Window::runRunnables()
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
		runnable(*this);
	}
};
void zg::Window::updateDeltaTime(NANO_TIMEPOINT now, bool updateLastFrameDeltaTime)
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
void zg::Window::resize(glm::vec2 newSize)
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
void zg::Window::registerOnEntityAddedFunction(const OnEntityAddedFunction& function)
{
	ZoneScoped;
	onEntityAdded = function;
	return;
}
uint32_t zg::Window::getScreenRefreshRate(uint32_t screenNum)
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
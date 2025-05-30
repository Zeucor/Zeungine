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
	title(true, info.title), childWindows([](auto& childWindow) { return *childWindow.title; }), windowWidth(true, info.windowWidth),
	windowHeight(true, info.windowHeight), windowX(true, info.windowX), windowY(true, info.windowY),
	scenes([](auto& scene) { return scene.name; }), deltaTime(true, 1.0 / info.framerate), borderless(info.borderless),
	framerate(true, info.framerate), vsync(info.vsync), frameduration(NANOSECONDS_DURATION(*deltaTime * NANOSECONDS::den)),
	framebudget(frameduration, 1, true, false), systemFonts(*this), postProcessingPipeline(INDEX_STACK)
{
	setViewport();
	ZGZoneScoped;
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
	deltaTime(other.deltaTime), borderless(other.borderless), framerate(other.framerate), vsync(other.vsync),
	frameduration(NANOSECONDS_DURATION(*deltaTime * NANOSECONDS::den)), framebudget(frameduration, 1, true, false), systemFonts(*this),
	postProcessingPipeline(INDEX_STACK),
	mainColorTexture(other.mainColorTexture),// mainDepthTexture(other.mainDepthTexture),
	mainFramebuffer(other.mainFramebuffer)
{
	setViewport();
	ZGZoneScoped;
	memset(windowKeys, 0, 256 * sizeof(int));
	memset(windowButtons, 0, 7 * sizeof(int));
}
zg::Window& zg::Window::operator=(const zg::Window& other)
{
	ZGZoneScoped;
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
	ZGZoneScoped;
// #if defined(_WIN32) || defined(__linux__)
// 	windowThread = std::make_shared<std::thread>(&zg::Window::startWindow, this);
// 	windowThread->join();
// #elif defined(MACOS)
	startWindow();
// #endif
}
void zg::Window::update()
{
	ZGZoneScopedN("zg::Window::update");
	auto componentsData = m_components.data();
	auto componentsSize = m_components.size();
	for (size_t index = 0; index < componentsSize; ++index)
	{
		ZGZoneScopedN("component:onUpdate");
		componentsData[index].onUpdate();
	}
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
	{
		ZGZoneScopedN("scene:update");
		scenesData[index].update();
	}
}
void zg::Window::preRender()
{
	ZGZoneScopedN("zg::Window::preRender");
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
	{
		ZGZoneScopedN("scene:preRender");
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
	ZGZoneScopedN("zg::Window::render");
	std::lock_guard lock(renderMutex);
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
	{
		ZGZoneScopedN("scene:render");
		scenesData[index].render();
	}
};
void zg::Window::postRender()
{
	ZGZoneScopedN("zg::Window::postRender");
	auto scenesData = scenes.data();
	auto scenesSize = scenes.size();
	for (size_t index = 0; index < scenesSize; ++index)
	{
		ZGZoneScopedN("scene:postRender");
		scenesData[index].postRender();
	}
	if (!isChildWindow)
		return;
	framebuffer->unbind();
}
void zg::Window::startWindow()
{
	ZGZoneScopedN("zg::Window::startWindow");
	iPlatformWindow = createPlatformWindow();
	auto& iPlatformWindowRef = *iPlatformWindow;
	iRenderer = createRenderer();
	auto& iRendererRef = *iRenderer;
	iPlatformWindowRef.init(*this);
	iRendererRef.createContext(&iPlatformWindowRef);
	iRendererRef.init();
	iPlatformWindowRef.postInit();
	fullscreenQuad = std::make_unique<FullscreenQuad>(INDEX_STACK, zg::shaders::RuntimeConstants({"ColorTexture"}));
	mainColorTexture = std::make_shared<textures::Texture>(iRenderer, glm::ivec4(*windowWidth, *windowHeight, 1, 0), (const void*)0, DEFAULT_TEXTURE_FORMAT, DEFAULT_TEXTURE_TYPE, DEFAULT_TEXTURE_FILTERTYPE, true, DEFAULT_TEXTURE_MULTISAMPLING);
	// mainDepthTexture = std::make_shared<textures::Texture>(iRenderer, glm::ivec4(windowWidth, windowHeight, 1, 0), (const void*)0, textures::Texture::Format::Depth, textures::Texture::Type::Float, textures::Texture::FilterType::Linear, true, textures::Texture::Multisampling::x1);
	mainFramebuffer = std::make_shared<textures::Framebuffer>(iRenderer, std::vector<textures::Framebuffer::TextureAttachmentPair>{
		{mainColorTexture, textures::Framebuffer::AttachmentType::Color}//,
		// {mainDepthTexture, textures::Framebuffer::AttachmentType::Depth}
	}, textures::BlendState::SrcAlpha);
	postProcessingPipeline.textureRegistry.registerOutput((std::numeric_limits<float>::lowest)(), "ColorTexture", mainColorTexture);
	// postProcessingPipeline.textureRegistry.registerOutput((std::numeric_limits<float>::lowest)(), "DepthTexture", mainDepthTexture);
	runRunnables();
	iPlatformWindowRef.disableKeyAutoRepeat();
	while (true)
	{
		ZGZoneScopedN("window:mainLoop");
		{
			ZGZoneScopedN("mainLoop:budgetBegin");
			auto _now = framebudget.begin();
			updateDeltaTime(_now, true);
		}
		{
			{
				ZGZoneScopedN("mainLoop:update");
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
			{
				ZGZoneScopedN("mainLoop:renderPass");
				preRender();
				render();
				ppOutputs.resize(scenes.size());
				auto index = 0;
				auto scenesData = scenes.data();
				auto scenesSize = scenes.size();
				for (size_t index = 0; index < scenesSize; ++index)
				{
					ZGZoneScopedN("p3:pp");
					auto& scene = scenesData[index];
					ppOutputs[index] = scene.postProcessingPipeline.postProcess();
				}
				index = 0;
				mainFramebuffer->bind();
				for (auto& sceneID : sortedScenes)
				{
					ZGZoneScopedN("p3/scene:render");
					auto& scene = Registry::GetSingleton().getScene(sceneID);
					scene.fsq->render(ppOutputs[index++]);
				}
				mainFramebuffer->unbind();
				{
					ZGZoneScopedN("mainLoop: p3:final");
					auto finalInputs = postProcessingPipeline.postProcess();
					{
						ZGZoneScopedN("mainLoop: final fsq render");
						iRendererRef.beginRenderPass();
						fullscreenQuad->render(finalInputs);
					}
					{
						ZGZoneScopedN("mainLoop:postRender");
						iRendererRef.postRenderPass();
						postRender();
					}
				}
			}
			{
				ZGZoneScopedN("mainLoop:budgetEnd");
				framebudget.end();
			}
		}
		ZGFrameMark;
		{
			ZGZoneScopedN("mainLoop:swapBuffers");
			callPreSwapbuffersOnceoff();
			iRendererRef.swapBuffers();
			framebudget.sleep();
		}
	}
}
void zg::Window::updateKeyboard()
{
	ZGZoneScoped;
	for (unsigned int i = 0; i < 256; ++i)
	{
		ZGZoneScoped;
		auto& pressed = windowKeys[i];
		if (keys[i] != pressed)
		{
			ZGZoneScoped;
			callKeyPressHandler(i, pressed);
			callAnyKeyPressHandler(i, pressed);
		}
		if (pressed)
		{
			ZGZoneScoped;
			callKeyUpdateHandler(i);
		}
	}
}
void zg::Window::updateMouse()
{
	ZGZoneScoped;
	for (unsigned int i = MinMouseButtonIndex; i < MaxMouseButtonIndex; ++i)
	{
		ZGZoneScoped;
	_checkPressed:
		auto& pressed = windowButtons[i];
		if (buttons[i] != pressed)
		{
			ZGZoneScoped;
			callMousePressHandler(i, pressed);
			if ((i == 3 || i == 4) && pressed)
			{
				ZGZoneScoped;
				windowButtons[i] = false;
				goto _checkPressed;
			}
		}
	}
	if (mouseMoved)
	{
		ZGZoneScoped;
		callMouseMoveHandler(newMouseCoords);
		mouseMoved = false;
	}
}
void zg::Window::close()
{
	ZGZoneScoped;
	if (isChildWindow)
	{
		ZGZoneScoped;
		return;
	}
	iPlatformWindow->close();
}
void zg::Window::minimize()
{
	ZGZoneScoped;
	minimized = true;
	maximized = false;
	if (isChildWindow)
	{
		ZGZoneScoped;
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
	ZGZoneScoped;
	minimized = false;
	if (maximized)
	{
		ZGZoneScoped;
		maximized = false;
		iPlatformWindow->restore();
		setXY(oldXY.x, oldXY.y);
	}
	else
	{
		ZGZoneScoped;
		maximized = true;
		iPlatformWindow->maximize();
		oldXY.x = *windowX;
		oldXY.y = *windowY;
		setXY(0, 0);
	}
}
void zg::Window::restore()
{
	ZGZoneScoped;
	minimized = false;
	maximized = false;
	if (isChildWindow)
	{
		ZGZoneScoped;
		return;
	}
	iPlatformWindow->restore();
	setXY(oldXY.x, oldXY.y);
}
void zg::Window::warpPointer(glm::vec2 coords)
{
	ZGZoneScoped;
	iPlatformWindow->warpPointer(coords);
	justWarpedPointer = true;
}
void zg::Window::setXY(float x, float y)
{
	ZGZoneScoped;
	windowX = x;
	windowY = y;
	if (isChildWindow)
	{
		ZGZoneScoped;
		return;
	}
	iPlatformWindow->setXY();
}
void zg::Window::setWidthHeight(float width, float height)
{
	ZGZoneScoped;
	windowWidth = width;
	windowHeight = height;
	setViewport();
	if (isChildWindow)
	{
		ZGZoneScoped;
		return;
	}
	iPlatformWindow->setWidthHeight();
}
void zg::Window::setViewport()
{
	viewport = {0, 0, *windowWidth, *windowHeight};
}
void zg::Window::mouseCapture(bool capture)
{
	ZGZoneScoped;
	iPlatformWindow->mouseCapture(capture);
}
zg::Window& zg::Window::createChildWindow(const WindowCreateInfo& info)
{
	ZGZoneScoped;
	auto usingInfo{info};
	usingInfo.isChildWindow = true;
	return *std::get<KEY_ID_VECTOR_VALUE_INDEX>(childWindows.emplace_back(usingInfo));
}

// Keyboard
UniqueIdentifier zg::Window::addKeyPressHandler(Key key, const KeyPressHandler& callback)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyPressHandlers[key];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void zg::Window::removeKeyPressHandler(Key key, UniqueIdentifier& id)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyPressHandlers[key];
	auto handlerIter = handlersPair.second.find(id);
	if (handlerIter == handlersPair.second.end())
	{
		ZGZoneScoped;
		return;
	}
	handlersPair.second.erase(handlerIter);
	id = 0;
};
UniqueIdentifier zg::Window::addKeyUpdateHandler(Key key, const KeyUpdateHandler& callback)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = keyUpdateHandlers[key];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void zg::Window::removeKeyUpdateHandler(Key key, UniqueIdentifier& id)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto handlersIter = keyUpdateHandlers.find(key);
	if (handlersIter == keyUpdateHandlers.end())
	{
		ZGZoneScoped;
		return;
	}
	auto& handlers = handlersIter->second.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		ZGZoneScoped;
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void zg::Window::callKeyPressHandler(Key key, int pressed)
{
	ZGZoneScoped;
	keys[key] = pressed;
	{
		std::vector<KeyPressHandler> handlersCopy;
		{
			// std::lock_guard lock(handlersMutex);
			auto handlersIter = keyPressHandlers.find(key);
			if (handlersIter == keyPressHandlers.end())
			{
				ZGZoneScoped;
				return;
			}
			auto& handlersMap = handlersIter->second.second;
			for (const auto& pair : handlersMap)
			{
				ZGZoneScoped;
				handlersCopy.push_back(pair.second);
			}
		}
		for (auto& handler : handlersCopy)
		{
			ZGZoneScoped;
			handler(!!pressed);
		}
	}
};
void zg::Window::callKeyUpdateHandler(Key key)
{
	ZGZoneScoped;
	std::vector<KeyUpdateHandler> handlersCopy;
	{
		// std::lock_guard lock(handlersMutex);
		auto handlersIter = keyUpdateHandlers.find(key);
		if (handlersIter == keyUpdateHandlers.end())
		{
			ZGZoneScoped;
			return;
		}
		auto& handlersMap = handlersIter->second.second;
		for (const auto& pair : handlersMap)
		{
			ZGZoneScoped;
			handlersCopy.push_back(pair.second);
		}
	}
	for (auto& handler : handlersCopy)
	{
		ZGZoneScoped;
		handler();
	}
};
UniqueIdentifier zg::Window::addAnyKeyPressHandler(const AnyKeyPressHandler& callback)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++anyKeyPressHandlers.first;
	anyKeyPressHandlers.second[id] = callback;
	return id;
};
void zg::Window::removeAnyKeyPressHandler(UniqueIdentifier& id)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlers = anyKeyPressHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		ZGZoneScoped;
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void zg::Window::callAnyKeyPressHandler(Key key, bool pressed)
{
	ZGZoneScoped;
	std::vector<AnyKeyPressHandler> handlersCopy;
	{
		ZGZoneScoped;
		// std::lock_guard lock(handlersMutex);
		auto& handlersMap = anyKeyPressHandlers.second;
		for (const auto& pair : handlersMap)
		{
			ZGZoneScoped;
			handlersCopy.push_back(pair.second);
		}
	}
	for (auto& handler : handlersCopy)
	{
		ZGZoneScoped;
		handler(key, pressed);
	}
}
void zg::Window::handleKey(Key key, int32_t mod, bool pressed)
{
	ZGZoneScoped;
	auto& window = *dynamic_cast<Window*>(this);
	bool hadChildFocus = false;
	auto childWindowsSize = childWindows.size();
	auto childWindowsData = childWindows.data();
	for (size_t index = 0; index < childWindowsSize; ++index)
	{
		ZGZoneScoped;
		auto& childWindow = childWindowsData[index];
		if (childWindow.minimized)
		{
			ZGZoneScoped;
			continue;
		}
		if (!childWindow.focused)
		{
			ZGZoneScoped;
			continue;
		}
		childWindow.mod = mod;
		childWindow.windowKeys[key] = pressed;
		hadChildFocus = true;
		break;
	}
	if (!hadChildFocus)
	{
		ZGZoneScoped;
		window.mod = mod;
		window.windowKeys[key] = pressed;
	}
}
// Mouse
UniqueIdentifier zg::Window::addMousePressHandler(Button button, const MousePressHandler& callback)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = mousePressHandlers[button];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void zg::Window::removeMousePressHandler(Button button, UniqueIdentifier& id)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlersPair = mousePressHandlers[button];
	auto handlerIter = handlersPair.second.find(id);
	if (handlerIter == handlersPair.second.end())
	{
		ZGZoneScoped;
		return;
	}
	handlersPair.second.erase(handlerIter);
	id = 0;
};
UniqueIdentifier zg::Window::addMouseMoveHandler(const MouseMoveHandler& callback)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++mouseMoveHandlers.first;
	mouseMoveHandlers.second[id] = callback;
	return id;
};
void zg::Window::removeMouseMoveHandler(UniqueIdentifier& id)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlers = mouseMoveHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		ZGZoneScoped;
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void zg::Window::callMousePressHandler(Button button, bool pressed)
{
	ZGZoneScoped;
	buttons[button] = pressed;
	{
		ZGZoneScoped;
		std::vector<MousePressHandler> handlersCopy;
		{
			ZGZoneScoped;
			// std::lock_guard lock(handlersMutex);
			auto handlersIter = mousePressHandlers.find(button);
			if (handlersIter == mousePressHandlers.end())
			{
				ZGZoneScoped;
				return;
			}
			auto& handlersMap = handlersIter->second.second;
			for (const auto& pair : handlersMap)
			{
				ZGZoneScoped;
				handlersCopy.push_back(pair.second);
			}
		}
		for (auto& handler : handlersCopy)
		{
			ZGZoneScoped;
			handler(!!pressed);
		}
	}
}
void zg::Window::callMouseMoveHandler(glm::vec2 coords)
{
	ZGZoneScoped;
	if (coords == mouseCoords)
	{
		ZGZoneScoped;
		return;
	}
	mouseCoords = coords;
	std::vector<MouseMoveHandler> handlersCopy;
	{
		ZGZoneScoped;
		// std::lock_guard lock(handlersMutex);
		auto& handlersMap = mouseMoveHandlers.second;
		for (const auto& pair : handlersMap)
		{
			ZGZoneScoped;
			handlersCopy.push_back(pair.second);
		}
	}
	for (auto& handler : handlersCopy)
	{
		ZGZoneScoped;
		handler(coords);
	}
}
void zg::Window::handleMouseMove(uint32_t x, uint32_t y)
{
	ZGZoneScoped;
	auto& window = *dynamic_cast<Window*>(this);
	bool hadChildFocus = false;
	auto childWindowsSize = childWindows.size();
	auto childWindowsData = childWindows.data();
	for (size_t index = 0; index < childWindowsSize; ++index)
	{
		ZGZoneScoped;
		auto& childWindow = childWindowsData[index];
		if (childWindow.minimized)
		{
			ZGZoneScoped;
			continue;
		}
		if (!childWindow.focused)
		{
			ZGZoneScoped;
			continue;
		}
		auto childX = x - *childWindow.windowX;
		auto childY = *childWindow.windowHeight - ((const float&)window.windowHeight - y - *childWindow.windowY);
		childWindow.newMouseCoords.x = childX, childWindow.newMouseCoords.y = childY;
		childWindow.mouseMoved = true;
		hadChildFocus = true;
		break;
	}
	if (!hadChildFocus)
	{
		ZGZoneScoped;
		window.newMouseCoords.y = y, window.newMouseCoords.x = x;
		window.mouseMoved = true;
	}
}
void zg::Window::handleMousePress(Button button, bool pressed)
{
	ZGZoneScoped;
	auto& window = *dynamic_cast<Window*>(this);
	bool hadChildFocus = false;
	auto childWindowsSize = childWindows.size();
	auto childWindowsData = childWindows.data();
	for (size_t index = 0; index < childWindowsSize; ++index)
	{
		ZGZoneScoped;
		auto& childWindow = childWindowsData[index];
		if (childWindow.minimized)
		{
			ZGZoneScoped;
			continue;
		}
		if (!childWindow.focused)
		{
			ZGZoneScoped;
			continue;
		}
		childWindow.windowButtons[button] = pressed;
		hadChildFocus = true;
		break;
	}
	if (!hadChildFocus)
	{
		ZGZoneScoped;
		window.windowButtons[button] = pressed;
	}
}
// resize
UniqueIdentifier zg::Window::addResizeHandler(const ViewResizeHandler& callback)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++viewResizeHandlers.first;
	viewResizeHandlers.second[id] = callback;
	return id;
};
void zg::Window::removeResizeHandler(UniqueIdentifier& id)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlers = viewResizeHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		ZGZoneScoped;
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void zg::Window::callResizeHandler(glm::vec2 newSize)
{
	ZGZoneScoped;
	std::vector<ViewResizeHandler> handlersCopy;
	{
		ZGZoneScoped;
		// std::lock_guard lock(handlersMutex);
		auto& handlersMap = viewResizeHandlers.second;
		for (const auto& pair : handlersMap)
		{
			ZGZoneScoped;
			handlersCopy.push_back(pair.second);
		}
	}
	for (auto& handler : handlersCopy)
	{
		ZGZoneScoped;
		handler(newSize);
	}
};
// focus
UniqueIdentifier zg::Window::addFocusHandler(const FocusHandler& callback)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto id = ++focusHandlers.first;
	focusHandlers.second[id] = callback;
	return id;
}
void zg::Window::removeFocusHandler(UniqueIdentifier& id)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	auto& handlers = focusHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		ZGZoneScoped;
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
}
void zg::Window::callFocusHandler(bool focused)
{
	ZGZoneScoped;
	if (this->focused == focused)
	{
		ZGZoneScoped;
		return;
	}
	std::vector<FocusHandler> handlersCopy;
	{
		ZGZoneScoped;
		// std::lock_guard lock(handlersMutex);
		auto& handlersMap = focusHandlers.second;
		for (const auto& pair : handlersMap)
		{
			ZGZoneScoped;
			handlersCopy.push_back(pair.second);
		}
	}
	this->focused = focused;
	for (auto& handler : handlersCopy)
	{
		ZGZoneScoped;
		handler(focused);
	}
}
// onceoffs
void zg::Window::addPreSwapbuffersOnceoff(const PreSwapbuffersOnceoff& onceoff)
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	preSwapbuffersOnceoffs.push(onceoff);
}
void zg::Window::callPreSwapbuffersOnceoff()
{
	ZGZoneScoped;
	// std::lock_guard lock(handlersMutex);
	if (preSwapbuffersOnceoffs.empty())
	{
		ZGZoneScoped;
		return;
	}
	auto onceoff = preSwapbuffersOnceoffs.front();
	preSwapbuffersOnceoffs.pop();
	onceoff();
}
size_t zg::Window::addShutdownHandler(const ShutdownHandler& handler)
{
	ZGZoneScoped;
	auto ID = GlobalUID::GetNew();
	shutdownHandlers[ID] = handler;
	return ID;
}
bool zg::Window::removeShutdownHandler(size_t& ID)
{
	ZGZoneScoped;
	auto iter = shutdownHandlers.find(ID);
	if (iter == shutdownHandlers.end())
	{
		ZGZoneScoped;
		return false;
	}
	shutdownHandlers.erase(iter);
	ID = 0;
	return true;
}
KeyIDVector<std::string, Scene>::EmplaceBackTuple zg::Window::addScene(const SceneCreateInfo& info)
{
	ZGZoneScoped;
	auto usingInfo{info};
	auto transaction = scenes.startTransaction();
	usingInfo.INDEX_STACK = {INDEX_STACK.begin(), INDEX_STACK.end()};
	usingInfo.INDEX_STACK.push_back(transaction.index);
	usingInfo.ID = transaction.id;
	usingInfo.INDEX = transaction.index;
	auto& scene = scenes.commitTransaction(transaction, usingInfo);
	Registry::GetSingleton().idScenes[scene.ID] = scene.INDEX_STACK;
	sceneZ = (scene.z = (sceneZ + 0.1f));
	if (scene.onAttachedFunction)
	{
		ZGZoneScoped;
		scene.onAttachedFunction(scene);
	}
	sortedScenes.push_back(scene.ID);
	sortScenes();
	return {transaction.key, transaction.id, transaction.index, &scene};
}
bool zg::Window::removeScene(size_t ID)
{
	ZGZoneScoped;
	auto iter = scenes.find_id(ID);
	if (iter == scenes.end())
	{
		ZGZoneScoped;
		return false;
	}
	auto& scene = *iter;
	if (scene.onDetachedFunction)
	{
		ZGZoneScoped;
		scene.onDetachedFunction(scene);
	}
	scene.onRemove();
	scene.detachAllComponents();
	sceneZ -= 0.1f;
	auto sortedSceneIter = std::find(sortedScenes.begin(), sortedScenes.end(), scene.ID);
	sortedScenes.erase(sortedSceneIter);
	scenes.erase(iter);
	sortScenes();
	auto& idScenes = Registry::GetSingleton().idScenes;
	auto idIter = idScenes.find(ID);
	if (idIter != idScenes.end())
	{
		ZGZoneScoped;
		idScenes.erase(idIter);
	}
	return true;
}
void zg::Window::sortScenes()
{
	std::sort(sortedScenes.begin(), sortedScenes.end(), [](auto& a_ID, auto& b_ID){
		auto& a_s = Registry::GetSingleton().getScene(a_ID);
		auto& b_s = Registry::GetSingleton().getScene(b_ID);
		return a_s.z > b_s.z;
	});
}
void zg::Window::runOnThread(const Runnable& runnable)
{
	ZGZoneScoped;
	// std::lock_guard lock(runnablesMutex);
	runnables.push(runnable);
};
void zg::Window::runRunnables()
{
	ZGZoneScoped;
	std::queue<Runnable> runnablesCopy;
	{
		ZGZoneScoped;
		// std::lock_guard lock(runnablesMutex);
		runnablesCopy = runnables;
		while (!runnables.empty())
		{
			ZGZoneScoped;
			runnables.pop();
		}
	}
	while (!runnablesCopy.empty())
	{
		ZGZoneScoped;
		auto runnable = runnablesCopy.front();
		runnablesCopy.pop();
		runnable(*this);
	}
};
void zg::Window::updateDeltaTime(NANO_TIMEPOINT now, bool updateLastFrameDeltaTime)
{
	ZGZoneScoped;
	if (updateLastFrameDeltaTime)
	{
		ZGZoneScoped;
		auto duration = now - lastFrameTime;
		lastFrameDeltaTime = duration.count() / 1'000'000'000.0L;
		auto& totalDeltaThisPeriodRef = *totalDeltaThisPeriod;
		totalDeltaThisPeriodRef += *lastFrameDeltaTime;
		((size_t&)totalFramesThisPeriod)++;
		static constexpr auto update_divider = 8.0;
		if (totalDeltaThisPeriodRef >= (1.0 / update_divider))
		{
			totalFramesLastPeriod = (*totalFramesThisPeriod) * update_divider;
			totalFramesThisPeriod = size_t(0);
			totalDeltaThisPeriod = long double(0.0L);
		}
	}
	lastFrameTime = now;
};
void zg::Window::resize(glm::vec2 newSize)
{
	ZGZoneScoped;
	if (windowWidth != newSize.x)
		windowWidth = newSize.x;
	if (windowHeight != newSize.y)
		windowHeight = newSize.y;
	auto scenesSize = scenes.size();
	auto scenesData = scenes.data();
	for (size_t index = 0; index < scenesSize; ++index)
	{
		ZGZoneScoped;
		scenesData[index].resize(newSize);
	}
	callResizeHandler(newSize);
};
void zg::Window::registerOnEntityAddedFunction(const OnEntityAddedFunction& function)
{
	ZGZoneScoped;
	onEntityAdded = function;
	return;
}
uint32_t zg::Window::getScreenRefreshRate(uint32_t screenNum)
{
	ZGZoneScoped;
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
void Window::onRemove()
{
	for (auto& scene : scenes)
		scene.onRemove();
	for (auto& pair : shutdownHandlers)
	{
		ZGZoneScopedN("shutdownHandler");
		pair.second(*this);
	}
	iPlatformWindow->enableKeyAutoRepeat();
	audioEngine.stop();
	audioEngine.clearPipeline();
	childWindows.clear();
	{
		auto scenesSize = scenes.size();
		auto scenesData = scenes.data();
		for (size_t i = 0; i < scenesSize; ++i)
		{
			ZGZoneScoped;
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
    iRenderer->destroy();
	delete iRenderer;
	iPlatformWindow->destroy();
	delete iPlatformWindow;
	zg::Entity::cleanupSerialize();
}
#pragma once
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include "components/windows/WindowComponent.hpp"
#include "ComponentHolder.hpp"
#include "fonts/SystemFonts.hpp"
#include "queue.hpp"
#include "system/Budget.hpp"
#include "./Events.hpp"
#include "./interfaces/IPlatformWindow.hpp"
#include "./textures/Framebuffer.hpp"
#include "audio/AudioEngine.hpp"
#include "Scene.hpp"
#include "PostProcessingPipeline.hpp"
#include "FullscreenQuad.hpp"
namespace zg
{
	namespace shaders
	{
		struct Shader;
	}
	namespace entities
	{
		struct Plane;
	}
	struct Scene;
#define KEYCODE_UP 17
#define KEYCODE_DOWN 18
#define KEYCODE_RIGHT 19
#define KEYCODE_LEFT 20
#define KEYCODE_HOME 0x80
#define KEYCODE_END 0x81
#define KEYCODE_PGUP 0x82
#define KEYCODE_PGDOWN 0x83
#define KEYCODE_INSERT 0x84
#define KEYCODE_NUMLOCK 0x85
#define KEYCODE_CAPSLOCK 0x86
#define KEYCODE_PAUSE 0x87
#define KEYCODE_SUPER 0x88
#define LAST_UNDEFINED_ASCII_IN_RANGE 0x9F
struct WindowCreateInfo;
	struct Window : ComponentHolder<Window, components::windows::WindowComponent, components::windows::WindowComponentCreateInfo>
	{
		size_t ID = 0;
		size_t* INDEX = 0;
		std::string title;
		IPlatformWindow* iPlatformWindow;
		IRenderer* iRenderer;
		float windowWidth;
		float windowHeight;
		float windowX;
		float windowY;
		uint32_t framerate = 60;
#if defined(_WIN32) || defined(__linux__)
		std::shared_ptr<std::thread> windowThread;
#endif
		std::recursive_mutex runnablesMutex;
		std::recursive_mutex handlersMutex;
		std::recursive_mutex renderMutex;
		std::queue<Runnable> runnables;
		std::unordered_map<Key, int> keys;
		std::unordered_map<Button, int> buttons;
		std::unordered_map<Key, std::pair<UniqueIdentifier, std::map<UniqueIdentifier, KeyPressHandler>>> keyPressHandlers;
		std::unordered_map<Key, std::pair<UniqueIdentifier, std::map<UniqueIdentifier, KeyUpdateHandler>>>
			keyUpdateHandlers;
		std::pair<UniqueIdentifier, std::map<UniqueIdentifier, AnyKeyPressHandler>> anyKeyPressHandlers;
		std::unordered_map<Button, std::pair<UniqueIdentifier, std::map<UniqueIdentifier, MousePressHandler>>>
			mousePressHandlers;
		std::pair<UniqueIdentifier, std::map<UniqueIdentifier, MouseMoveHandler>> mouseMoveHandlers;
		std::pair<UniqueIdentifier, std::map<UniqueIdentifier, ViewResizeHandler>> viewResizeHandlers;
		std::pair<UniqueIdentifier, std::map<UniqueIdentifier, FocusHandler>> focusHandlers;
		zg::td::queue<PreSwapbuffersOnceoff> preSwapbuffersOnceoffs;
		zg::KeyIDVector<std::string, Scene> scenes;
		bool open = true;
		NANO_TIMEPOINT lastFrameTime;
		long double deltaTime;
		long double lastFrameDeltaTime;
		bool justWarpedPointer = false;
		bool borderless = false;
		bool minimized = false;
		bool maximized = false;
		bool focused = false;
		OnEntityAddedFunction onEntityAdded;
		int windowKeys[256];
		int windowButtons[7];
		bool mouseMoved = false;
		glm::vec2 mouseCoords;
		int mod = 0;
		bool isChildWindow = false;
		Window* parentWindow = 0;
		Scene* parentScene = 0;
		KeyIDVector<std::string, Window> childWindows;
		bool NDCFramebufferPlane;
		std::shared_ptr<textures::Texture> framebufferTexture;
		std::shared_ptr<textures::Texture> framebufferDepthTexture;
		std::shared_ptr<textures::Framebuffer> framebuffer;
		Entity* framebufferPlane;
		glm::vec2 oldXY;
		bool vsync = true;
		audio::AudioEngine audioEngine;
		NANOSECONDS_DURATION frameduration;
		budget::ZBudget<SYS_CLOCK, NANO_TIMEPOINT, NANOSECONDS_DURATION, LD_REAL> framebudget;
		fonts::SystemFonts systemFonts;
		PostProcessingPipeline postProcessingPipeline;
		std::unique_ptr<FullscreenQuad> fullscreenQuad;
		// when adding new members remember to add to operator=

		Window(const WindowCreateInfo& info);
		Window(const Window& other);
		~Window();
		Window& operator=(const Window& other);
		void run();
		void update();
		void preRender();
		void render();
		void postRender();
		void startWindow();
		void updateKeyboard();
		void updateMouse();
		void close();
		void minimize();
		void maximize();
		void restore();
		void warpPointer(glm::vec2 coords);
		void setXY(float x, float y);
		void setWidthHeight(float width, float height);
		void mouseCapture(bool capture);
		Window& createChildWindow(const WindowCreateInfo& info);
		// Keyboard
		UniqueIdentifier addKeyPressHandler(Key key, const KeyPressHandler& callback);
		void removeKeyPressHandler(Key key, UniqueIdentifier& id);
		UniqueIdentifier addKeyUpdateHandler(Key key, const KeyUpdateHandler& callback);
		void removeKeyUpdateHandler(Key key, UniqueIdentifier& id);
		UniqueIdentifier addAnyKeyPressHandler(const AnyKeyPressHandler& callback);
		void removeAnyKeyPressHandler(UniqueIdentifier& id);
		void callKeyPressHandler(Key key, int pressed);
		void callKeyUpdateHandler(Key key);
		void callAnyKeyPressHandler(Key key, bool pressed);
		void handleKey(Key key, int32_t mod, bool pressed);
		// Mouse
		UniqueIdentifier addMousePressHandler(Button button, const MousePressHandler& callback);
		void removeMousePressHandler(Button button, UniqueIdentifier& id);
		UniqueIdentifier addMouseMoveHandler(const MouseMoveHandler& callback);
		void removeMouseMoveHandler(UniqueIdentifier& id);
		void callMousePressHandler(Button button, int pressed);
		void callMouseMoveHandler(glm::vec2 coords);
		void handleMouseMove(uint32_t x, uint32_t y);
		void handleMousePress(Button button, bool pressed);
		// resize
		UniqueIdentifier addResizeHandler(const ViewResizeHandler& callback);
		void removeResizeHandler(UniqueIdentifier& id);
		void callResizeHandler(glm::vec2 newSize);
		// focus
		UniqueIdentifier addFocusHandler(const FocusHandler& callback);
		void removeFocusHandler(UniqueIdentifier& id);
		void callFocusHandler(bool focused);
		// preSwapbuffers
		void addPreSwapbuffersOnceoff(const PreSwapbuffersOnceoff& onceoff);
		void callPreSwapbuffersOnceoff();
		// scene
		KeyIDVector<std::string, Scene>::EmplaceBackTuple  addScene(const SceneCreateInfo& info);
		bool removeScene(size_t sceneID);
		// runnables
		void runOnThread(const Runnable& runnable);
		void runRunnables();
		void updateDeltaTime(NANO_TIMEPOINT now, bool updateLastFrameDeltaTime);
		void resize(glm::vec2 newSize);
		void registerOnEntityAddedFunction(const OnEntityAddedFunction& function);
	};
	void computeNormals(zg::FRONTFACE frontFace, const std::vector<uint32_t>& indices,
											const std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals);
	struct WindowCreateInfo
	{
		std::string title = "Default Window Name";
		float windowWidth = 1024;
		float windowHeight = 768;
		float windowX = -1;
		float windowY = -1;
		bool borderless = false;
		bool vsync = true;
		uint32_t framerate = 60;
		bool isChildWindow = false;
		Window* parentWindowPointer = 0;
		Scene* parentScenePointer = 0;
		bool NDCFramebufferPlane = false;
	};
} // namespace zg

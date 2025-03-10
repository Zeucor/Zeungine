#pragma once
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include "./Events.hpp"
#include "./interfaces/IPlatformWindow.hpp"
#include "./textures/Framebuffer.hpp"
#include "audio/AudioEngine.hpp"
#include <zg/queue.hpp>
#include <zg/system/Budget.hpp>
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
	struct ShaderContext
	{
		std::unordered_map<uint32_t, std::shared_ptr<shaders::Shader>> shaders;
		std::unordered_map<std::size_t, std::pair<uint32_t, std::shared_ptr<shaders::Shader>>> shadersByHash;
		uint32_t shaderCount = 0;
	};
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
	struct Window
	{
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
		std::mutex runnablesMutex;
		std::queue<Runnable> runnables;
		std::unordered_map<Key, int> keys;
		std::unordered_map<Button, int> buttons;
		std::mutex handlersMutex;
		std::unordered_map<Key, std::pair<EventIdentifier, std::map<EventIdentifier, KeyPressHandler>>> keyPressHandlers;
		std::unordered_map<Key, std::pair<EventIdentifier, std::map<EventIdentifier, KeyUpdateHandler>>> keyUpdateHandlers;
		std::pair<EventIdentifier, std::map<EventIdentifier, AnyKeyPressHandler>> anyKeyPressHandlers;
		std::unordered_map<Button, std::pair<EventIdentifier, std::map<EventIdentifier, MousePressHandler>>>
			mousePressHandlers;
		std::pair<EventIdentifier, std::map<EventIdentifier, MouseMoveHandler>> mouseMoveHandlers;
		std::pair<EventIdentifier, std::map<EventIdentifier, ViewResizeHandler>> viewResizeHandlers;
		std::pair<EventIdentifier, std::map<EventIdentifier, FocusHandler>> focusHandlers;
		zg::td::queue<PreSwapbuffersOnceoff> preSwapbuffersOnceoffs;
		std::shared_ptr<Scene> scene;
		bool open = true;
		std::chrono::steady_clock::time_point lastFrameTime;
		long double deltaTime;
		bool justWarpedPointer = false;
		bool borderless = false;
		bool minimized = false;
		bool maximized = false;
		bool focused = false;
		OnEntityAddedFunction onEntityAdded;
		std::recursive_mutex renderMutex;
		const char* title;
		int windowKeys[256];
		int windowButtons[7];
		bool mouseMoved = false;
		glm::vec2 mouseCoords;
		int mod = 0;
		bool isChildWindow = false;
		Window* parentWindow = 0;
		Scene* parentScene = 0;
		std::vector<Window*> childWindows;
		ShaderContext* shaderContext = 0;
		bool NDCFramebufferPlane;
		std::shared_ptr<textures::Texture> framebufferTexture;
		std::shared_ptr<textures::Texture> framebufferDepthTexture;
		std::shared_ptr<textures::Framebuffer> framebuffer;
		std::shared_ptr<entities::Plane> framebufferPlane;
		glm::vec2 oldXY;
		bool vsync = true;
		audio::AudioEngine audioEngine;
		NANOSECONDS_DURATION frameduration;
		budget::ZBudget<SYS_CLOCK, NANO_TIMEPOINT, NANOSECONDS_DURATION, LD_REAL> framebudget;
		Window(const char* title, float windowWidth, float windowHeight, float windowX, float windowY,
					 bool borderless = false, bool vsync = true, uint32_t framerate = 60);
		Window(Window& parentWindow, Scene& parentScene, const char* childTitle, float childWindowWidth,
					 float childWindowHeight, float childWindowX, float childWindowY, bool NDCFramebufferPlane = false,
					 bool vsync = true, uint32_t framerate = 60);
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
		Window& createChildWindow(const char* title, Scene& scene, float windowWidth, float windowHeight, float windowX,
															float windowY, bool NDCFramebufferPlane);
		// Keyboard
		EventIdentifier addKeyPressHandler(Key key, const KeyPressHandler& callback);
		void removeKeyPressHandler(Key key, EventIdentifier& id);
		EventIdentifier addKeyUpdateHandler(Key key, const KeyUpdateHandler& callback);
		void removeKeyUpdateHandler(Key key, EventIdentifier& id);
		EventIdentifier addAnyKeyPressHandler(const AnyKeyPressHandler& callback);
		void removeAnyKeyPressHandler(EventIdentifier& id);
		void callKeyPressHandler(Key key, int pressed);
		void callKeyUpdateHandler(Key key);
		void callAnyKeyPressHandler(Key key, bool pressed);
		void handleKey(Key key, int32_t mod, bool pressed);
		// Mouse
		EventIdentifier addMousePressHandler(Button button, const MousePressHandler& callback);
		void removeMousePressHandler(Button button, EventIdentifier& id);
		EventIdentifier addMouseMoveHandler(const MouseMoveHandler& callback);
		void removeMouseMoveHandler(EventIdentifier& id);
		void callMousePressHandler(Button button, int pressed);
		void callMouseMoveHandler(glm::vec2 coords);
		void handleMouseMove(uint32_t x, uint32_t y);
		void handleMousePress(Button button, bool pressed);
		// resize
		EventIdentifier addResizeHandler(const ViewResizeHandler& callback);
		void removeResizeHandler(EventIdentifier& id);
		void callResizeHandler(glm::vec2 newSize);
		// focus
		EventIdentifier addFocusHandler(const FocusHandler& callback);
		void removeFocusHandler(EventIdentifier& id);
		void callFocusHandler(bool focused);
		// preSwapbuffers
		void addPreSwapbuffersOnceoff(const PreSwapbuffersOnceoff& onceoff);
		void callPreSwapbuffersOnceoff();
		// scene
		std::shared_ptr<Scene> setScene(const std::shared_ptr<Scene>& scene);
		// runnables
		void runOnThread(const Runnable& runnable);
		void runRunnables();
		void updateDeltaTime();
		void resize(glm::vec2 newSize);
		void registerOnEntityAddedFunction(const OnEntityAddedFunction& function);
	};
	void computeNormals(const std::vector<uint32_t>& indices, const std::vector<glm::vec3>& positions,
											std::vector<glm::vec3>& normals);
} // namespace zg

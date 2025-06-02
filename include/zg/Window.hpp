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
#include "EventExecutor.hpp"
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
	struct Registry;
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
#define KEYCODE_CTRL 0x87
#define KEYCODE_SHIFT 0x88
#define KEYCODE_ALT 0x89
#define KEYCODE_PAUSE 0x87
#define KEYCODE_SUPER 0x88
#define LAST_UNDEFINED_ASCII_IN_RANGE 0x9F
struct WindowCreateInfo;
	struct Window :
		ComponentHolder<Window, components::windows::WindowComponent, components::windows::WindowComponentCreateInfo>,
		EventExecutor
	{
		friend Registry;
		size_t ID = 0;
		size_t* INDEX = 0;
		std::vector<size_t*> INDEX_STACK;
		observable_ptr<std::string> title;
		IPlatformWindow* iPlatformWindow;
		IRenderer* iRenderer;
		observable_ptr<float> windowWidth;
		observable_ptr<float> windowHeight;
		observable_ptr<glm::vec4> viewport = observable_ptr<glm::vec4>(true, glm::vec4(0));
		observable_ptr<float> windowX;
		observable_ptr<float> windowY;
		observable_ptr<uint32_t> framerate = observable_ptr<uint32_t>(true, uint32_t(60));
		observable_ptr<long double> smoothedDeltaTime;
		observable_ptr<long double> smoothingFactor;
		observable_ptr<float> fps = observable_ptr<float>(new float(0.0f));
		size_t summingDrawCount = 0;
		observable_ptr<size_t> drawCount = observable_ptr<size_t>(new size_t(0)); 
		size_t summingTriangleCount = 0;
		observable_ptr<size_t> triangleCount = observable_ptr<size_t>(new size_t(0)); 
		float sceneZ = 0.0f;
		std::vector<size_t> sortedScenes;
#if defined(_WIN32) || defined(__linux__)
		std::shared_ptr<std::thread> windowThread;
#endif
		std::recursive_mutex runnablesMutex;
		std::recursive_mutex handlersMutex;
		std::recursive_mutex renderMutex;
		std::queue<Runnable> runnables;
		zg::td::queue<PreSwapbuffersOnceoff> preSwapbuffersOnceoffs;
		zg::KeyIDVector<std::string, Scene> scenes;
		bool open = true;
		NANO_TIMEPOINT lastFrameTime;
		observable_ptr<long double> deltaTime;
		observable_ptr<long double> lastFrameDeltaTime;
		observable_ptr<long double> lastTotalDeltaTIme;
		observable_ptr<long double> totalDeltaThisPeriod;
		observable_ptr<size_t> totalFramesThisPeriod;
		observable_ptr<size_t> totalFramesLastPeriod;
		bool justWarpedPointer = false;
		bool borderless = false;
		bool minimized = false;
		bool maximized = false;
		bool focused = false;
		OnEntityAddedFunction onEntityAdded;
		bool windowKeys[256];
		bool windowButtons[7];
		bool mouseMoved = false;
		glm::vec2 mouseCoords;
		glm::vec2 newMouseCoords;
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
		std::shared_ptr<textures::Texture> mainColorTexture;
		// std::shared_ptr<textures::Texture> mainDepthTexture;
		std::shared_ptr<textures::Framebuffer> mainFramebuffer;
		PostProcessingPipeline postProcessingPipeline;
		std::unique_ptr<FullscreenQuad> fullscreenQuad;
		std::vector<std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>>> ppOutputs;
		// when adding new members remember to add to operator=

		Window(const WindowCreateInfo& info);
		Window(const Window& other);
		Window& operator=(const Window& other);
		void run();
		void update();
		void preRender();
		void render();
		void postRender();
		void startWindow();
		void close();
		void minimize();
		void maximize();
		void restore();
		void warpPointer(glm::vec2 coords);
		void setXY(float x, float y);
		void setWidthHeight(float width, float height);
		void setViewport();
		void mouseCapture(bool capture);
		Window& createChildWindow(const WindowCreateInfo& info);
		// focus
		void queueFocusEvent(bool focused);
		// preSwapbuffers
		void addPreSwapbuffersOnceoff(const PreSwapbuffersOnceoff& onceoff);
		void callPreSwapbuffersOnceoff();
		// scene
		KeyIDVector<std::string, Scene>::EmplaceBackTuple  addScene(const SceneCreateInfo& info);
		bool removeScene(size_t ID);
		void sortScenes();
		// runnables
		void runOnThread(const Runnable& runnable);
		void runRunnables();
		void updateDeltaTime(NANO_TIMEPOINT now, bool updateLastFrameDeltaTime);
		void resize(glm::vec2 newSize);
		void registerOnEntityAddedFunction(const OnEntityAddedFunction& function);
		static uint32_t getScreenRefreshRate(uint32_t screenNum);
	protected:
		void onRemove();
	};
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
		bool NDCFramebufferPlane = false;
		size_t ID;
		size_t* INDEX;
		std::vector<size_t*> INDEX_STACK;
	};
} // namespace zg

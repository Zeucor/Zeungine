#pragma once
#include <thread>
#include <functional>
#include <map>
#include <queue>
#include "./IScene.hpp"
#include "./glm.hpp"
namespace anex
{
	struct IWindow
	{
		int32_t windowWidth;
		int32_t windowHeight;
		int32_t windowX;
		int32_t windowY;
		uint32_t framerate = 60;
		std::shared_ptr<std::thread> windowThread;
		using Runnable = std::function<void(IWindow&)>;
		std::queue<Runnable> runnables;
		using Key = uint32_t;
		using Button = uint32_t;
		using EventIdentifier = uint32_t;
		using KeyPressHandler = std::function<void(const bool &)>;
		using KeyUpdateHandler = std::function<void()>;
		using MousePressHandler = std::function<void(const bool &)>;
		using MouseMoveHandler = std::function<void(const glm::vec2 &)>;
		std::unordered_map<Key, int> keys;
		std::unordered_map<Button, int> buttons;
		std::unordered_map<Key, std::pair<EventIdentifier, std::map<EventIdentifier, KeyPressHandler>>> keyPressHandlers;
		std::unordered_map<Key, std::pair<EventIdentifier, std::map<EventIdentifier, KeyUpdateHandler>>> keyUpdateHandlers;
		std::unordered_map<Button, std::pair<EventIdentifier, std::map<EventIdentifier, MousePressHandler>>> mousePressHandlers;
		std::pair<EventIdentifier, std::map<EventIdentifier, MouseMoveHandler>> mouseMoveHandlers;
		std::shared_ptr<IScene> scene;
		bool open = true;
		glm::vec4 clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		std::chrono::steady_clock::time_point lastFrameTime;
		float deltaTime = 0;
		bool justWarpedPointer = false;
		bool borderless = false;
		bool minimized = false;
		bool maximized = false;
		bool focused = false;
		using OnEntityAddedFunction = std::function<void(const std::shared_ptr<IEntity> &)>;
		OnEntityAddedFunction onEntityAdded;
		IWindow(const int32_t &windowWidth,
						const int32_t &windowHeight,
						const int32_t &windowX,
						const int32_t &windowY,
						const bool &borderless,
						const uint32_t &framerate);
		virtual ~IWindow() = default;
		void run();
		void awaitWindowThread() const;
		virtual void startWindow() = 0;
		virtual void preRender();
		void render();
		virtual void postRender();
		// Keyboard
		virtual void updateKeyboard() = 0;
		EventIdentifier addKeyPressHandler(const Key &key, const KeyPressHandler &callback);
		void removeKeyPressHandler(const Key &key, EventIdentifier &id);
		EventIdentifier addKeyUpdateHandler(const Key &key, const KeyUpdateHandler &callback);
		void removeKeyUpdateHandler(const Key &key, EventIdentifier &id);
		void callKeyPressHandler(const Key &key, const int &pressed);
		void callKeyUpdateHandler(const Key &key);
		// Mouse
		virtual void updateMouse() = 0;
		EventIdentifier addMousePressHandler(const Button &button, const MousePressHandler &callback);
		void removeMousePressHandler(const Button &button, EventIdentifier &id);
		EventIdentifier addMouseMoveHandler(const MouseMoveHandler &callback);
		void removeMouseMoveHandler(EventIdentifier &id);
		void callMousePressHandler(const Button &button, const int &pressed);
		void callMouseMoveHandler(const glm::vec2 &coords);
		std::shared_ptr<IScene> setIScene(const std::shared_ptr<IScene> &scene);
		void runOnThread(const Runnable &runnable);
		void runRunnables();
		void updateDeltaTime();
		void resize(const glm::vec2 &newSize);
		void registerOnEntityAddedFunction(const OnEntityAddedFunction &function);
		virtual void close();
		virtual void minimize();
		virtual void maximize();
		virtual void restore();
		virtual void drawLine(int x0, int y0, int x1, int y1, uint32_t color) = 0;
		virtual void drawRectangle(int x, int y, int w, int h, uint32_t color) = 0;
		virtual void drawCircle(int x, int y, int radius, uint32_t color) = 0;
		virtual void drawText(int x, int y, const char* text, int scale, uint32_t color) = 0;
		virtual void warpPointer(const glm::vec2 &coords);
		virtual void setXY(const int32_t &x, const int32_t &y);
		virtual void setWidthHeight(const uint32_t &width, const uint32_t &height);
		virtual IWindow &createChildWindow(const char *title, IScene &scene, const uint32_t &windowWidth, const uint32_t &windowHeight, const int32_t &windowX, const int32_t &windowY);
	};
}
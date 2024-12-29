#pragma once
#include <thread>
#include <functional>
#include <map>
#include <queue>

#include "./IScene.hpp"
#include <glm/glm.hpp>
namespace anex
{
	struct IWindow
	{
		int windowWidth;
		int windowHeight;
		int framerate = 60;
		std::shared_ptr<std::thread> windowThread;
		using Runnable = std::function<void(IWindow&)>;
		std::queue<Runnable> runnables;
		std::unordered_map<unsigned int, int> keys;
		std::unordered_map<unsigned int, std::pair<unsigned int, std::map<unsigned int, std::function<void(const bool &)>>>> keyHandlers;
		std::unordered_map<unsigned int, std::pair<unsigned int, std::map<unsigned int, std::function<void()>>>> keyUpdateHandlers;
		std::shared_ptr<IScene> scene;
		bool open = true;
		glm::vec4 clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		IWindow(const int &windowWidth, const int &windowHeight, const int &framerate);
		virtual ~IWindow() = default;
		void run();
		void awaitWindowThread();
		virtual void startWindow() = 0;
		void render();
		virtual void updateKeys() = 0;
		unsigned int addKeyHandler(const unsigned int &key, const std::function<void(const bool &)> &callback);
		void removeKeyHandler(const unsigned int &key, unsigned int &id);
		unsigned int addKeyUpdateHandler(const unsigned int &key, const std::function<void()> &callback);
		void removeKeyUpdateHandler(const unsigned int &key, unsigned int &id);
		void callKeyPressHandler(const unsigned int &key, const int &pressed);
		void callKeyUpdateHandler(const unsigned int &key);
		std::shared_ptr<IScene> setIScene(const std::shared_ptr<IScene> &scene);
		void runOnThread(const Runnable &runnable);
		void runRunnables();
		virtual void close() = 0;
		virtual void drawLine(int x0, int y0, int x1, int y1, uint32_t color) = 0;
		virtual void drawRectangle(int x, int y, int w, int h, uint32_t color) = 0;
		virtual void drawCircle(int x, int y, int radius, uint32_t color) = 0;
		virtual void drawText(int x, int y, const char* text, int scale, uint32_t color) = 0;
	};
}
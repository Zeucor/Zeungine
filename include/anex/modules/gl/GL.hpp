#pragma once
#include <anex/IWindow.hpp>
#include <anex/modules/gl/shaders/Shader.hpp>
#include <anex/modules/gl/vaos/VAO.hpp>
#include "./common.hpp"
namespace anex::modules::gl
{
	struct GLWindow : IWindow
	{
		const char *title;
#ifdef _WIN32
		HWND hwnd;
		HDC hDeviceContext;
		HGLRC hRenderingContext;
#endif
		int windowKeys[256];
		int windowButtons[5];
		bool mouseMoved = false;
		glm::vec2 mouseCoords;
		int mod = 0;
		GLWindow(const char *title, const int &windowWidth, const int &windowHeight, const int &framerate = 60);
		~GLWindow();
		void startWindow() override;
		void renderInit();
		void updateKeyboard() override;
		void updateMouse() override;
		void close() override;
		void drawLine(int x0, int y0, int x1, int y1, uint32_t color) override;
		void drawRectangle(int x, int y, int w, int h, uint32_t color) override;
		void drawCircle(int x, int y, int radius, uint32_t color) override;
		void drawText(int x, int y, const char* text, int scale, uint32_t color) override;
	};
}
#pragma once
#include <zg/IWindow.hpp>
struct fenster;
namespace zg::modules::fenster
{
	struct FensterWindow : IWindow
	{
    std::shared_ptr<uint32_t> buf;
    struct fenster *f = 0;
		bool mouseMoved = false;
		int oldMouse = 0;
		glm::vec2 oldMousePos = glm::vec2(0);
		int framerate = 60;
		FensterWindow(const char *title,
									float windowWidth,
									float windowHeight,
									float windowX,
									float windowY,
									bool borderless = false,
									uint32_t framerate = 60);
		~FensterWindow();
		void startWindow() override;
		void updateKeyboard() override;
		void updateMouse() override;
		void close() override;
		void drawLine(int x0, int y0, int x1, int y1, uint32_t color) override;
		void drawRectangle(int x, int y, int w, int h, uint32_t color) override;
		void drawCircle(int x, int y, int radius, uint32_t color) override;
		void drawText(int x, int y, const char* text, int scale, uint32_t color) override;
	};
	void fenster_line(struct fenster* f, int x0, int y0, int x1, int y1, uint32_t c);
	void fenster_rect(struct fenster* f, int x, int y, int w, int h, uint32_t c);
	void fenster_circle(struct fenster* f, int x, int y, int r, uint32_t c);
	void fenster_fill(struct fenster* f, int x, int y, uint32_t old, uint32_t c);
	void fenster_text(struct fenster* f, int x, int y, const char* s, int scale, uint32_t c);
	std::pair<int, int> fenster_text_bounds(const char* s, int scale);
}
#pragma once
#include <anex/IWindow.hpp>
struct fenster;
namespace anex::modules::fenster
{
	struct FensterWindow : IWindow
	{
	    std::shared_ptr<uint32_t> buf;
	    struct fenster *f = 0;
		int framerate = 60;
		FensterWindow(const char *title, const int &windowWidth, const int &windowHeight, const int &framerate = 60);
		~FensterWindow();
		void startWindow() override;
		void updateKeys() override;
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
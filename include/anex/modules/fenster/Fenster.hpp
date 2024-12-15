#pragma once
#include <anex/IGame.hpp>
struct fenster;
namespace anex::modules::fenster
{
	struct FensterGame : IGame
	{
    std::shared_ptr<uint32_t> buf;
    struct fenster *f = 0;
		FensterGame(const int &windowWidth, const int &windowHeight);
		~FensterGame();
		void startWindow() override;
		void updateKeys() override;
		void close() override;
	};
	void fenster_line(struct fenster* f, int x0, int y0, int x1, int y1, uint32_t c);
	void fenster_rect(struct fenster* f, int x, int y, int w, int h, uint32_t c);
	void fenster_circle(struct fenster* f, int x, int y, int r, uint32_t c);
	void fenster_fill(struct fenster* f, int x, int y, uint32_t old, uint32_t c);
	void fenster_text(struct fenster* f, int x, int y, const char* s, int scale, uint32_t c);
	std::pair<int, int> fenster_text_bounds(const char* s, int scale);
}
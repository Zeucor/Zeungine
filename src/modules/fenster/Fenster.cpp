#include <zg/modules/fenster/Fenster.hpp>
#include <fenster.hpp>
using namespace zg::modules::fenster;
FensterWindow::FensterWindow(const char* title,
                             float windowWidth,
                             float windowHeight,
                             float windowX,
                             float windowY,
                             bool borderless,
                             uint32_t framerate):
	IWindow(windowWidth, windowHeight, windowX, windowY, borderless, framerate),
	buf((uint32_t*)malloc(windowWidth * windowHeight * sizeof(uint32_t)), free),
	f(new struct fenster({title, (int)windowWidth, (int)windowHeight, buf.get()}))
{
  run();
};
FensterWindow::~FensterWindow()
{
	delete f;
};
void FensterWindow::startWindow()
{
	fenster_open(f);
	uint32_t t = 0;
	int64_t now = fenster_time();
	while (fenster_loop(f) == 0)
	{
	  runRunnables();
		updateKeyboard();
		fenster_rect(f, 0, 0, windowWidth, windowHeight, 0x00000000);
		render();
		int64_t time = fenster_time();
		if (time - now < 1000 / framerate)
			fenster_sleep(time - now);
		now = time;
	}
};
void FensterWindow::updateKeyboard()
{
	for (unsigned int i = 0; i < 256; ++i)
	{
		int& pressed = f->keys[i];
		if (keys[i] != pressed)
		{
			callKeyPressHandler(i, pressed);
			callAnyKeyPressHandler(i, pressed);
		}
		if (pressed)
		{
		  callKeyUpdateHandler(i);
		}
	}
};
void FensterWindow::updateMouse()
{
  if (f->mouse != oldMouse)
  {
    oldMouse = f->mouse;
    int pressed = oldMouse;
    if (buttons[0] != pressed)
    {
      callMousePressHandler(0, pressed);
    }
  }
  glm::vec2 mousePos = {f->x, f->y};
  if (mousePos != oldMousePos)
  {
    mouseMoved = true;
    oldMousePos = mousePos;
  }
  if (mouseMoved)
  {
    callMouseMoveHandler({f->x, f->y});
    mouseMoved = false;
  }
};
void FensterWindow::close()
{
	open = false;
	fenster_close(f);
};
void FensterWindow::drawLine(int x0, int y0, int x1, int y1, uint32_t color)
{
  fenster_line(f, x0, y0, x1, y1, color);
};
void FensterWindow::drawRectangle(int x, int y, int w, int h, uint32_t color)
{
  fenster_rect(f, x, y, w, h, color);
};
void FensterWindow::drawCircle(int x, int y, int radius, uint32_t color)
{
  fenster_circle(f, x, y, radius, color);
};
void FensterWindow::drawText(int x, int y, const char* text, int scale, uint32_t color)
{
  fenster_text(f, x, y, text, scale, color);
};
void zg::modules::fenster::fenster_line(struct fenster* f, int x0, int y0, int x1, int y1,
                  uint32_t c)
{
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2, e2;
  for (;;)
  {
    fenster_pixel(f, x0, y0) = c;
    if (x0 == x1 && y0 == y1)
    {
      break;
    }
    e2 = err;
    if (e2 > -dx)
    {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy)
    {
      err += dx;
      y0 += sy;
    }
  }
};
void zg::modules::fenster::fenster_rect(struct fenster* f, int x, int y, int w, int h,
                  uint32_t c)
{
  for (int row = 0; row < h; row++)
  {
    for (int col = 0; col < w; col++)
    {
      fenster_pixel(f, x + col, y + row) = c;
    }
  }
};
void zg::modules::fenster::fenster_circle(struct fenster* f, int x, int y, int r, uint32_t c)
{
  for (int dy = -r; dy <= r; dy++)
  {
    for (int dx = -r; dx <= r; dx++)
    {
      if (dx * dx + dy * dy <= r * r && x + dx >= 0 && y + dy >= 0 && x + dx < f->width && y + dy < f->height)
      {
        fenster_pixel(f, x + dx, y + dy) = c;
      }
    }
  }
};
void zg::modules::fenster::fenster_fill(struct fenster* f, int x, int y, uint32_t old,
                  uint32_t c)
{
  if (x < 0 || y < 0 || x >= f->width || y >= f->height)
  {
    return;
  }
  if (fenster_pixel(f, x, y) == old)
  {
    fenster_pixel(f, x, y) = c;
    fenster_fill(f, x - 1, y, old, c);
    fenster_fill(f, x + 1, y, old, c);
    fenster_fill(f, x, y - 1, old, c);
    fenster_fill(f, x, y + 1, old, c);
  }
};
// clang-format off
uint16_t font5x3[] = {0x0000,0x2092,0x002d,0x5f7d,0x279e,0x52a5,0x7ad6,0x0012,0x4494,0x1491,0x017a,0x05d0,0x1400,0x01c0,0x0400,0x12a4,0x2b6a,0x749a,0x752a,0x38a3,0x4f4a,0x38cf,0x3bce,0x12a7,0x3aae,0x49ae,0x0410,0x1410,0x4454,0x0e38,0x1511,0x10e3,0x73ee,0x5f7a,0x3beb,0x624e,0x3b6b,0x73cf,0x13cf,0x6b4e,0x5bed,0x7497,0x2b27,0x5add,0x7249,0x5b7d,0x5b6b,0x3b6e,0x12eb,0x4f6b,0x5aeb,0x388e,0x2497,0x6b6d,0x256d,0x5f6d,0x5aad,0x24ad,0x72a7,0x6496,0x4889,0x3493,0x002a,0xf000,0x0011,0x6b98,0x3b79,0x7270,0x7b74,0x6750,0x95d6,0xb9ee,0x5b59,0x6410,0xb482,0x56e8,0x6492,0x5be8,0x5b58,0x3b70,0x976a,0xcd6a,0x1370,0x38f0,0x64ba,0x3b68,0x2568,0x5f68,0x54a8,0xb9ad,0x73b8,0x64d6,0x2492,0x3593,0x03e0};
// clang-format on
void zg::modules::fenster::fenster_text(struct fenster* f, int x, int y, const char* s, int scale,
                  uint32_t c)
{
  while (*s)
  {
    char chr = *s++;
    if (chr > 32)
    {
      uint16_t bmp = font5x3[chr - 32];
      for (int dy = 0; dy < 5; dy++)
      {
        for (int dx = 0; dx < 3; dx++)
        {
          if (bmp >> (dy * 3 + dx) & 1)
          {
            fenster_rect(f, x + dx * scale, y + dy * scale, scale, scale, c);
          }
        }
      }
    }
    x = x + 4 * scale;
  }
};
std::pair<int, int> zg::modules::fenster::fenster_text_bounds(const char* s, int scale)
{
  int x = 0, y = 5 * scale;
  while (*s)
  {
    char chr = *s++;
    x += 4 * scale;
  }
  x -= scale;
  return {x, y};
};
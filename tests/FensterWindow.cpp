#include <zg/modules/fenster/Fenster.hpp>
using namespace zg::modules::fenster;
struct TestTextEntity : zg::IEntity
{
  TestTextEntity(zg::IWindow& window):
    IEntity(window)
  {};
  void render() override
  {
    window.drawText(0, 0, "Test Text", 5, 0x00FFFFFF);
  };
};
struct TestRectangleEntity : zg::IEntity
{
  TestRectangleEntity(zg::IWindow& window):
    IEntity(window)
  {};
  void render() override
  {
    window.drawRectangle(0, 0, window.windowWidth, window.windowHeight, 0x0000FF00);
  };
};
struct TestCircleEntity : zg::IEntity
{
  TestCircleEntity(zg::IWindow& window):
    IEntity(window)
  {};
  void render() override
  {
    window.drawCircle(window.windowWidth / 2, window.windowHeight / 2, 10, 0x00FF0000);
  };
};
struct TestScene : zg::IScene
{
  TestScene(zg::IWindow& window):
    zg::IScene(window)
  {
    addEntity(std::make_shared<TestRectangleEntity>(window));
    addEntity(std::make_shared<TestCircleEntity>(window));
    addEntity(std::make_shared<TestTextEntity>(window));
  };
  void preRender() override {};
};
int main()
{
  FensterWindow window("FensterWindow", 640, 480);
  window.setIScene(std::make_shared<TestScene>(window));
  std::this_thread::sleep_for(std::chrono::seconds(5));
  window.close();
  window.awaitWindowThread();
};
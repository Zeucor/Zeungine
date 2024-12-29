#include <anex/modules/fenster/Fenster.hpp>
using namespace anex::modules::fenster;
struct TestTextEntity : anex::IEntity
{
  TestTextEntity(anex::IWindow& window):
    IEntity(window)
  {};
  void render() override
  {
    window.drawText(0, 0, "Test Text", 5, 0x00FFFFFF);
  };
};
struct TestRectangleEntity : anex::IEntity
{
  TestRectangleEntity(anex::IWindow& window):
    IEntity(window)
  {};
  void render() override
  {
    window.drawRectangle(0, 0, window.windowWidth, window.windowHeight, 0x0000FF00);
  };
};
struct TestCircleEntity : anex::IEntity
{
  TestCircleEntity(anex::IWindow& window):
    IEntity(window)
  {};
  void render() override
  {
    window.drawCircle(window.windowWidth / 2, window.windowHeight / 2, 10, 0x00FF0000);
  };
};
struct TestScene : anex::IScene
{
  TestScene(anex::IWindow& window):
    anex::IScene(window)
  {
    addEntity(std::make_shared<TestRectangleEntity>(window));
    addEntity(std::make_shared<TestCircleEntity>(window));
    addEntity(std::make_shared<TestTextEntity>(window));
  };
};
int main()
{
  FensterWindow window("FensterWindow", 640, 480);
  window.setIScene(std::make_shared<TestScene>(window));
  std::this_thread::sleep_for(std::chrono::seconds(5));
  window.close();
  window.awaitWindowThread();
};
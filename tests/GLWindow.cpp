#include <anex/modules/gl/GL.hpp>
#include <array>
using namespace anex::modules::gl;
struct TestScene : anex::IScene
{
  TestScene(anex::IWindow &window);
};
struct TestTriangle : anex::IEntity, vaos::VAO
{
  shaders::Shader shader;
  uint32_t indices[3];
  std::array<glm::vec4, 3> colors;
  std::array<glm::vec3, 3> positions;
  TestTriangle(anex::IWindow &window):
    IEntity(window),
    VAO({"Color", "Position"}, 3),
    shader(constants),
    indices(0, 1, 2),
    colors({{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}}),
    positions({{0, 1, 0}, {1, -1, 0}, {-1, -1, 0}})
  {
    updateIndices(indices);
    updateElements("Color", colors.data());
    updateElements("Position", positions.data());
  };
  void render() override
  {
    shader.use(true);
    vaoDraw();
    shader.use(false);
  };
};
TestScene::TestScene(anex::IWindow& window):
  IScene(window)
{
  addEntity(std::make_shared<TestTriangle>(window));
};
int main()
{
  GLWindow window("GLWindow", 640, 480);
  window.clearColor = {1, 0, 0, 1};
  window.runOnThread([](auto &window)
  {
    window.setIScene(std::make_shared<TestScene>(window));
  });
  std::this_thread::sleep_for(std::chrono::seconds(20));
  window.close();
  window.awaitWindowThread();
};
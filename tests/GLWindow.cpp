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
  glm::mat4 position;
  glm::mat4 rotation;
  float rotationAmount = 0;
  TestTriangle(anex::IWindow &window):
    IEntity(window),
    VAO({"Color", "Position", "View", "Projection", "Model"}, 3),
    shader(constants),
    indices(0, 1, 2),
    colors({{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}}),
    positions({{0, 100, 0}, {100, -100, 0}, {-100, -100, 0}}),
    position(glm::translate(glm::mat4(1.0f), glm::vec3(0, 5, 0))),
    rotation(glm::mat4(1.0f))
  {
    updateIndices(indices);
    updateElements("Color", colors.data());
    updateElements("Position", positions.data());
    shader.setBlock("View", glm::lookAt({0, 0, 10}, {0, 0, 0}, glm::vec3{0, 1, 0}));
    shader.setBlock("Projection", glm::ortho(0.f, (float)window.windowWidth,
                                      0.f, (float)window.windowHeight,
                                      0.1f, 100.f));
    window.addMouseMoveHandler([&](const auto &coords)
    {
      position = glm::translate(glm::mat4(1.0f), glm::vec3(coords.x, coords.y, 0));
    });
    window.addMousePressHandler(3, [&](const auto &pressed)
    {
      colors[0] = pressed ? glm::vec4(1, 1, 1, 1) : glm::vec4(1, 0, 0, 1);
      updateElements("Color", colors.data());
    });
    window.addMousePressHandler(4, [&](const auto &pressed)
    {
      colors[1] = pressed ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 1, 0, 1);
      updateElements("Color", colors.data());
    });
  };
  void render() override
  {
    // rotationAmount += glm::radians(0.01f);
    // rotation = glm::rotate(glm::mat4(1.0f), rotationAmount, glm::vec3(0, 0, 1));
    auto model = position * rotation;
    shader.setBlock("Model", model);
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
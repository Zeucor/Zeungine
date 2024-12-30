#include <anex/modules/gl/GL.hpp>
#include <array>
using namespace anex::modules::gl;
struct TestScene : anex::IScene
{
  glm::vec3 cameraPosition;
  glm::mat4 view;
  glm::mat4 projection;
  TestScene(anex::IWindow &window);
  void updateView();
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
  TestScene &testScene;
  TestTriangle(anex::IWindow &window, TestScene &testScene):
    IEntity(window),
    VAO({"Color", "Position", "View", "Projection", "Model", "Fog", "CameraPosition"}, 3),
    shader(constants),
    indices(2, 1, 0),
    colors({{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}}),
    positions({{0, 100, 0}, {100, -100, 0}, {-100, -100, 0}}),
    position(glm::translate(glm::mat4(1.0f), glm::vec3(0, 100, 0))),
    rotation(glm::mat4(1.0f)),
    testScene(testScene)
  {
    updateIndices(indices);
    updateElements("Color", colors.data());
    updateElements("Position", positions.data());
    shader.use(true);
    shader.setUniform("fogDensity", 0.0035f);
    shader.setUniform("fogColor", glm::vec4(1, 1, 1, 1));
    shader.use(false);
    window.addMouseMoveHandler([&](const auto &coords)
    {
      position = glm::translate(glm::mat4(1.0f), glm::vec3(coords.x, coords.y, 0));
    });
    window.addMousePressHandler(5, [&](const auto &pressed)
    {
      colors[0] = pressed ? glm::vec4(1, 1, 1, 1) : glm::vec4(1, 0, 0, 1);
      updateElements("Color", colors.data());
    });
    window.addMousePressHandler(6, [&](const auto &pressed)
    {
      colors[1] = pressed ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 1, 0, 1);
      updateElements("Color", colors.data());
    });
    window.addMousePressHandler(0, [&](const auto &pressed)
    {
      colors[2] = pressed ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 0, 1, 1);
      updateElements("Color", colors.data());
    });
  };
  void render() override
  {
    // rotationAmount += glm::radians(0.01f);
    // rotation = glm::rotate(glm::mat4(1.0f), rotationAmount, glm::vec3(0, 0, 1));
    auto model = position * rotation;
    shader.use(true);
    shader.setBlock("Model", model);
    shader.setBlock("View", testScene.view);
    shader.setBlock("Projection", testScene.projection);
    shader.setUniform("CameraPosition", testScene.cameraPosition);
    vaoDraw();
    shader.use(false);
  };
};
struct TestCube : anex::IEntity, vaos::VAO
{
  shaders::Shader shader;
  uint32_t indices[36]; // 6 faces * 2 triangles * 3 vertices
  std::array<glm::vec4, 24> colors;
  std::array<glm::vec3, 24> positions;
  glm::vec3 position;
  glm::mat4 rotation;
  float rotationAmount = 0;
  TestScene &testScene;

  TestCube(anex::IWindow &window, TestScene &testScene)
    : IEntity(window),
      VAO({"Color", "Position", "View", "Projection", "Model", "Fog", "CameraPosition"}, 36),
      shader(constants),
      indices{
        0, 1, 2,  2, 3, 0,   // Front face
        4, 7, 6,  6, 5, 4,   // Back face
        8, 9, 10, 10, 11, 8,  // Left face
        12, 15, 14, 14, 13, 12, // Right face
        16, 17, 18, 18, 19, 16, // Top face
        20, 23, 22, 22, 21, 20  // Bottom face
      },
      colors({{1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, // Front face
              {0, 1, 0, 1}, {0, 1, 0, 1}, {0, 1, 0, 1}, {0, 1, 0, 1}, // Back face
              {0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, // Left face
              {1, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 0, 1}, // Right face
              {0, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1}, // Top face
              {1, 0, 1, 1}, {1, 0, 1, 1}, {1, 0, 1, 1}, {1, 0, 1, 1} // Bottom face
             }),
      positions({
        { -50, -50,  50}, {  50, -50,  50}, {  50,  50,  50}, { -50,  50,  50}, // Front
        { -50, -50, -50}, {  50, -50, -50}, {  50,  50, -50}, { -50,  50, -50}, // Back
        { -50, -50, -50}, { -50, -50,  50}, { -50,  50,  50}, { -50,  50, -50}, // Left
        {  50, -50, -50}, {  50, -50,  50}, {  50,  50,  50}, {  50,  50, -50}, // Right
        { -50,  50, -50}, { -50,  50,  50}, {  50,  50,  50}, {  50,  50, -50}, // Top
        { -50, -50, -50}, { -50, -50,  50}, {  50, -50,  50}, {  50, -50, -50}  // Bottom
      }),
      position(window.windowWidth / 2, window.windowHeight / 2, 0),
      rotation(glm::mat4(1.0f)),
      testScene(testScene)
  {
    updateIndices(indices);
    updateElements("Color", colors.data());
    updateElements("Position", positions.data());
    shader.use(true);
    shader.setUniform("fogDensity", 0.0035f);
    shader.setUniform("fogColor", glm::vec4(1, 1, 1, 1));
    shader.use(false);
    window.addMousePressHandler(3, [&](const auto &pressed)
    {
      if (pressed)
        position.z -= 10.f;
    });
    window.addMousePressHandler(4, [&](const auto &pressed)
    {
      if (pressed)
        position.z += 10.f;
    });
  };

  void render() override
  {
    rotationAmount += glm::radians(0.01f);
    rotation = glm::rotate(glm::mat4(1.0f), rotationAmount, glm::vec3(0, 1, 0));
    auto model = glm::translate(glm::mat4(1.0f), position) * rotation;
    shader.use(true);
    shader.setBlock("Model", model);
    shader.setBlock("View", testScene.view);
    shader.setBlock("Projection", testScene.projection);
    shader.setUniform("CameraPosition", testScene.cameraPosition);
    vaoDraw();
    shader.use(false);
  };
};
TestScene::TestScene(anex::IWindow& window):
  IScene(window),
  cameraPosition(window.windowWidth / 2, window.windowHeight / 2 - window.windowHeight / 2, window.windowWidth),
  // projection(glm::ortho(0.f, (float)window.windowWidth, 0.f, (float)window.windowHeight,0.1f, 100.f))
  projection(glm::perspective(glm::radians(81.f), (float)window.windowWidth / window.windowHeight, 0.1f, 10000.f))
{
  addEntity(std::make_shared<TestTriangle>(window, *this));
  addEntity(std::make_shared<TestCube>(window, *this));
  window.addKeyUpdateHandler(20, [&]()
  {
    cameraPosition.x -= 500.f * window.deltaTime;
    updateView();
  });
  window.addKeyUpdateHandler(19, [&]()
  {
    cameraPosition.x += 500.f * window.deltaTime;
    updateView();
  });
  window.addKeyUpdateHandler(17, [&]()
  {
    cameraPosition.y += 500.f * window.deltaTime;
    updateView();
  });
  window.addKeyUpdateHandler(18, [&]()
  {
    cameraPosition.y -= 500.f * window.deltaTime;
    updateView();
  });
  updateView();
};
void TestScene::updateView()
{
  view = glm::lookAt(cameraPosition, {window.windowWidth / 2, window.windowHeight / 2, 0}, glm::vec3{0, 1, 0});
}

int main()
{
  GLWindow window("GLWindow", 640, 480);
  window.clearColor = {0, 0, 0, 1};
  window.runOnThread([](auto &window)
  {
    window.setIScene(std::make_shared<TestScene>(window));
  });
  std::this_thread::sleep_for(std::chrono::seconds(20));
  window.close();
  window.awaitWindowThread();
};
#include <zg/Window.hpp>
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/lights/Lights.hpp>
#include <zg/lights/DirectionalLight.hpp>
#include <zg/Logger.hpp>
#include <array>
#include <zg/images/ImageLoader.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/entities/SkyBox.hpp>
#include <zg/vp/VML.hpp>
using namespace zg;
struct TestTriangle;
struct TestScene : Scene
{
  std::shared_ptr<entities::Cube> cubeEntity;
  vp::VML vml;
  explicit TestScene(IWindow &_window);
};
struct TestTriangle : Entity
{
  std::vector<glm::vec4> colors;
  std::vector<glm::vec3> normals = {};
  float rotationAmount = 0;
  TestScene &testScene;
  TestTriangle(IWindow &_window, TestScene &testScene, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale):
    Entity(_window, {
      "Color",
      "Position",
      "Normal",
      "View",
      "Projection",
      "Model",
      "CameraPosition",
      "Lighting",
      "DirectionalLightShadowMaps",
      "SpotLightShadowMaps",
      "PointLightShadowMaps",
      "LightSpacePosition",
      "Fog"
    }, 3, {2, 1, 0}, 3, {{0, 100, 0}, {100, -100, 0}, {-100, -100, 0}}, position, rotation, scale, "TestTriangle"),
    colors({{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}}),
    testScene(testScene)
  {
    computeNormals(indices, positions, normals);
    updateIndices(indices);
    updateElements("Color", colors);
    updateElements("Position", positions);
    updateElements("Normal", normals);
    window.addMousePressHandler(5, [&](const auto &pressed)
    {
      colors[0] = pressed ? glm::vec4(1, 1, 1, 1) : glm::vec4(1, 0, 0, 1);
      updateElements("Color", colors);
    });
    window.addMousePressHandler(6, [&](const auto &pressed)
    {
      colors[1] = pressed ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 1, 0, 1);
      updateElements("Color", colors);
    });
    window.addMousePressHandler(0, [&](const auto &pressed)
    {
      colors[2] = pressed ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 0, 1, 1);
      updateElements("Color", colors);
    });
  };
  void preRender() override
  {
    auto &glWindow = (Window &)window;
    auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(glWindow.iVendorRenderer);
  	glRenderer.glContext->Disable(GL_CULL_FACE);
    GLcheck(glRenderer, "glEnable");
    const auto &model = getModelMatrix();
    shader.bind();
    testScene.entityPreRender(*this);
    shader.setBlock("Model", model);
    shader.setBlock("View", testScene.view.matrix);
    shader.setBlock("Projection", testScene.projection.matrix);
    shader.setBlock("CameraPosition", testScene.view.position, 16);
    shader.unbind();
  };
  void postRender() override
  {
    auto &glWindow = (Window &)window;
    auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(glWindow.iVendorRenderer);
    glRenderer.glContext->Enable(GL_CULL_FACE);
    GLcheck(glRenderer, "glEnable");
  };
};
void rotateLightPosition(IWindow &window, Scene &scene, lights::PointLight &light, lights::PointLightShadow &pointLightShadow, float angleSpeed, shaders::Shader &shader)
{
  window.runOnThread([&, angleSpeed](auto &window) mutable
  {
    auto &deltaTime = window.deltaTime;
    auto adjustedDeltaTime = (std::min)(1.0f / 144.f, deltaTime);
    auto adjustedCurrentAngleSpeed = adjustedDeltaTime * angleSpeed;
    static const glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), adjustedCurrentAngleSpeed, glm::vec3(0, 1, 0));
    glm::vec4 newPosition = rotationMatrix * glm::vec4(light.position, 1.0f);
    light.position = glm::vec3(newPosition);
    pointLightShadow.updateShadowTransforms();
    shader.bind();
    shader.setSSBO("PointLights", scene.pointLights.data(), scene.pointLights.size() * sizeof(lights::PointLight));
    shader.unbind();
    window.runOnThread([&, angleSpeed](auto &window)
    {
      rotateLightPosition(window, scene, light, pointLightShadow, angleSpeed, shader);
    });
  });
};
TestScene::TestScene(IWindow& window):
  Scene(window, { 0, 10, 10}, glm::normalize(glm::vec3(0, -1, -1)), 81.f),
  vml(*this)
{
  pointLights.push_back({
    {5, 10, 0},
    {1, 1, 1},
    1.0,
    10000,
    1.f,
    250.f
  });
  pointLightShadows.emplace_back(dynamic_cast<Window &>(window), pointLights[0]);
  directionalLights.push_back({
    {0, 25, 20}, // position
    glm::normalize(glm::vec3(0, -1, -1)), // direction
    {1, 1, 1}, // color
    0.5, // intensity,
    1.f, // nearPlane
    250.f // farPlane
  });
  directionalLightShadows.emplace_back(dynamic_cast<Window &>(window), directionalLights[0]);
  spotLights.push_back({
    {0, 25, -20}, // position
    glm::normalize(glm::vec3(0, -1, 1)), // direction
    {0.0f, 0.0f, 1.0f}, // color
    1.0f, // intensity
    glm::cos(glm::radians(25.0f)), // cutoff
    glm::cos(glm::radians(50.0f)), // outerCutoff
    1.f,
    250.f
  });
  spotLightShadows.emplace_back(dynamic_cast<Window &>(window), spotLights[0]);
  cubeEntity = std::make_shared<entities::Cube>((Window &)window, *this,
    glm::vec3(0, 3, 0), // position
    glm::vec3(0, 0, 0), // rotation
    glm::vec3(1, 1, 1), // scale
    glm::vec3(1, 1, 1),
    shaders::RuntimeConstants({
      "Lighting",
      "DirectionalLightShadowMaps",
      "SpotLightShadowMaps",
      "PointLightShadowMaps",
      "LightSpacePosition",
      "Fog"
    }));
  addEntity(cubeEntity); // size
  addEntity(std::make_shared<entities::Cube>((Window &)window, *this,
    glm::vec3(0, 0, 0),
    glm::vec3(0, 0, 0),
    glm::vec3(1, 1, 1),
    glm::vec3(10, 1, 10),
    shaders::RuntimeConstants({
      "Lighting",
      "DirectionalLightShadowMaps",
      "SpotLightShadowMaps",
      "PointLightShadowMaps",
      "LightSpacePosition",
      "Fog"
    })));
  auto &shader = cubeEntity->shader;
  rotateLightPosition(window, *this, pointLights[0], pointLightShadows[0], glm::radians(180.0f), shader);
  shader.bind();
  shader.setSSBO("PointLights", pointLights.data(), pointLights.size() * sizeof(lights::PointLight));
  shader.setSSBO("DirectionalLights", directionalLights.data(), directionalLights.size() * sizeof(lights::DirectionalLight));
  shader.setSSBO("SpotLights", spotLights.data(), spotLights.size() * sizeof(lights::SpotLight));
  shader.setUniform("fogDensity", 0.035f);
  shader.setUniform("fogColor", glm::vec4(0, 0, 0, 1));
  shader.unbind();
  auto skybox = std::make_shared<entities::SkyBox>(
    (Window &)window,
    *this,
    std::vector<std::string_view>({
      "images/skybox/right.jpg", "images/skybox/left.jpg", "images/skybox/top.jpg",
      "images/skybox/bottom.jpg", "images/skybox/front.jpg", "images/skybox/back.jpg"
    })
  );
  addEntity(skybox);
  window.addKeyUpdateHandler(20, [&]()
  {
    view.position.x -= 1.f * window.deltaTime;
    view.update();
  });
  window.addKeyUpdateHandler(19, [&]()
  {
    view.position.x += 1.f * window.deltaTime;
    view.update();
  });
  window.addKeyUpdateHandler(17, [&]()
  {
    view.position.y += 1.f * window.deltaTime;
    view.update();
  });
  window.addKeyUpdateHandler(18, [&]()
  {
    view.position.y -= 1.f * window.deltaTime;
    view.update();
  });
  window.addKeyPressHandler(27, [&](const auto &pressed)
  {
    if (!pressed)
      window.close();
  });
};

int main()
{
  Window window("Window", 1280, 720, -1, -1);
  window.clearColor = {0, 0, 0, 1};
  window.runOnThread([](auto &window)
  {
    window.setIScene(std::make_shared<TestScene>(window));
  });
  window.await();
};
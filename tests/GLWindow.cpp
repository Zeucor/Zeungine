#include <anex/modules/gl/GL.hpp>
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/lights/Lights.hpp>
#include <anex/modules/gl/lights/DirectionalLight.hpp>
#include <anex/Logger.hpp>
#include <array>
#include <anex/images/ImageLoader.hpp>
using namespace anex::modules::gl;
struct TestTriangle;
struct TestScene : anex::modules::gl::GLScene
{
  std::shared_ptr<TestTriangle> triangleEntity;
  explicit TestScene(anex::IWindow &window);
};
struct TestTriangle : anex::modules::gl::GLEntity
{
  uint32_t indices[3];
  std::array<glm::vec4, 3> colors;
  std::array<glm::vec3, 3> positions;
  std::array<glm::vec3, 3> normals = {};
  float rotationAmount = 0;
  TestScene &testScene;
  TestTriangle(anex::IWindow &window, TestScene &testScene, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale):
    GLEntity(window, {
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
    }, 3, position, rotation, scale),
    indices(2, 1, 0),
    colors({{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}}),
    positions({{0, 100, 0}, {100, -100, 0}, {-100, -100, 0}}),
    testScene(testScene)
  {
    computeNormals(3, indices, positions, normals);
    updateIndices(indices);
    updateElements("Color", colors.data());
    updateElements("Position", positions.data());
    updateElements("Normal", normals.data());
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
  void preRender() override
  {
    glDisable(GL_CULL_FACE);
    GLcheck("glEnable");
    const auto &model = getModelMatrix();
    shader.bind();
    testScene.entityPreRender(*this);
    shader.setBlock("Model", model);
    shader.setBlock("View", testScene.view);
    shader.setBlock("Projection", testScene.projection);
    shader.setBlock("CameraPosition", testScene.cameraPosition, 16);
    shader.unbind();
  };
  void postRender() override
  {
    glEnable(GL_CULL_FACE);
    GLcheck("glEnable");
  };
};
struct TestCube : anex::modules::gl::GLEntity
{
  uint32_t indices[36]; // 6 faces * 2 triangles * 3 vertices
  std::array<glm::vec4, 24> colors;
  std::array<glm::vec3, 24> positions;
  std::array<glm::vec3, 24> normals = {};
  TestScene &testScene;
  bool mouseMove;
  float oscillationParam = 0;
  glm::vec3 startPosition;

  TestCube(anex::IWindow &window, TestScene &testScene, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const glm::vec3 &size, const bool &mouseMove = false):
    GLEntity(window, {
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
    }, 36, position, rotation, scale),
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
      { -size.x / 2, -size. y / 2,  size.z / 2}, {  size.x / 2, -size. y / 2,  size.z / 2}, {  size.x / 2,  size. y / 2,  size.z / 2}, { -size.x / 2,  size. y / 2,  size.z / 2}, // Front
      { -size.x / 2, -size. y / 2, -size.z / 2}, {  size.x / 2, -size. y / 2, -size.z / 2}, {  size.x / 2,  size. y / 2, -size.z / 2}, { -size.x / 2,  size. y / 2, -size.z / 2}, // Back
      { -size.x / 2, -size. y / 2, -size.z / 2}, { -size.x / 2, -size. y / 2,  size.z / 2}, { -size.x / 2,  size. y / 2,  size.z / 2}, { -size.x / 2,  size. y / 2, -size.z / 2}, // Left
      {  size.x / 2, -size. y / 2, -size.z / 2}, {  size.x / 2, -size. y / 2,  size.z / 2}, {  size.x / 2,  size. y / 2,  size.z / 2}, {  size.x / 2,  size. y / 2, -size.z / 2}, // Right
      { -size.x / 2,  size. y / 2, -size.z / 2}, { -size.x / 2,  size. y / 2,  size.z / 2}, {  size.x / 2,  size. y / 2,  size.z / 2}, {  size.x / 2,  size. y / 2, -size.z / 2}, // Top
      { -size.x / 2, -size. y / 2, -size.z / 2}, { -size.x / 2, -size. y / 2,  size.z / 2}, {  size.x / 2, -size. y / 2,  size.z / 2}, {  size.x / 2, -size. y / 2, -size.z / 2}  // Bottom
    }),
    testScene(testScene),
    mouseMove(mouseMove),
    startPosition(position)
  {
    computeNormals(36, indices, positions, normals);
    updateIndices(indices);
    updateElements("Color", colors.data());
    updateElements("Position", positions.data());
    updateElements("Normal", normals.data());
    // if (mouseMove)
    //   window.addMouseMoveHandler([&](const auto &coords)
    //   {
    //     this->position = {0, coords.y / window.windowHeight, 0};
    //   });
  };

  void preRender() override
  {
    if (mouseMove)
    {
      static constexpr float rotationSpeed = glm::radians(90.0f);
      rotation.y += rotationSpeed * window.deltaTime;
      if (rotation.y > glm::two_pi<float>())
      {
        rotation.y -= glm::two_pi<float>();
      }
      oscillateBetween(startPosition, startPosition + glm::vec3(5, 0, 0), window.deltaTime);
    }
    const auto &model = getModelMatrix();
    shader.bind();
    testScene.entityPreRender(*this);
    shader.setBlock("Model", model);
    shader.setBlock("View", testScene.view);
    shader.setBlock("Projection", testScene.projection);
    shader.setBlock("CameraPosition", testScene.cameraPosition, 16);
    shader.unbind();
  };
  void oscillateBetween(const glm::vec3 &positionA, const glm::vec3 &positionB, float deltaTime)
  {
    constexpr float oscillationSpeed = 1.0f;
    oscillationParam += oscillationSpeed * (std::min)(deltaTime, 1.0f / 144.f);
    if (oscillationParam > 2.0f)
    {
      oscillationParam -= 2.0f;
    }
    if (oscillationParam <= 1.0f)
    {
      position = glm::mix(positionA, positionB, oscillationParam);
    }
    else
    {
      position = glm::mix(positionB, positionA, oscillationParam - 1.0f);
    }
  };
};
void rotateLightPosition(anex::IWindow &window, GLScene &scene, lights::PointLight &light, lights::PointLightShadow &pointLightShadow, float angleSpeed, shaders::Shader &shader)
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
TestScene::TestScene(anex::IWindow& window):
  GLScene(window, { 5, 10, 10}, glm::normalize(glm::vec3(0, -1, -1)), 81.f)
{
  pointLights.push_back({
    {5, 10, 0},
    {1, 1, 1},
    1.0,
    10000,
    1.f,
    250.f
  });
  pointLightShadows.emplace_back(pointLights[0]);
  directionalLights.push_back({
    {0, 25, 20}, // position
    glm::normalize(glm::vec3(0, -1, -1)), // direction
    {1, 1, 1}, // color
    0.5, // intensity,
    1.f, // nearPlane
    250.f // farPlane
  });
  directionalLightShadows.emplace_back(directionalLights[0]);
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
  spotLightShadows.emplace_back(spotLights[0]);
  // auto imagePair = anex::images::ImageLoader::load("images/skybox/back.jpg");
  textures::Texture skyboxTexture(
    {0, 0, 0, 1},
    {
      {
        "images/skybox/left.jpg", "images/skybox/right.jpg", "images/skybox/top.jpg",
        "images/skybox/bottom.jpg", "images/skybox/front.jpg", "images/skybox/back.jpg"
      }
    }
  );
  triangleEntity = std::dynamic_pointer_cast<TestTriangle>(std::make_shared<TestTriangle>(window, *this, glm::vec3(1, 5, -1), glm::vec3(0, glm::radians(90.f), 0), glm::vec3(1, 1, 1)));
  // addEntity(triangleEntity);
  addEntity(std::make_shared<TestCube>(window, *this,
    glm::vec3(0, 3, 0), // position
    glm::vec3(0, 0, 0), // rotation
    glm::vec3(1, 1, 1), // scale
    glm::vec3(1, 1, 1), true)); // size
  addEntity(std::make_shared<TestCube>(window, *this, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec3(250, 1, 250)));
  auto &shader = triangleEntity->shader;
  rotateLightPosition(window, *this, pointLights[0], pointLightShadows[0], glm::radians(180.0f), shader);
  shader.bind();
  shader.setSSBO("PointLights", pointLights.data(), pointLights.size() * sizeof(lights::PointLight));
  shader.setSSBO("DirectionalLights", directionalLights.data(), directionalLights.size() * sizeof(lights::DirectionalLight));
  shader.setSSBO("SpotLights", spotLights.data(), spotLights.size() * sizeof(lights::SpotLight));
  shader.setUniform("fogDensity", 0.035f);
  shader.setUniform("fogColor", glm::vec4(0, 0, 0, 1));
  shader.unbind();
  window.addKeyUpdateHandler(20, [&]()
  {
    cameraPosition.x -= 1.f * window.deltaTime;
    updateView();
  });
  window.addKeyUpdateHandler(19, [&]()
  {
    cameraPosition.x += 1.f * window.deltaTime;
    updateView();
  });
  window.addKeyUpdateHandler(17, [&]()
  {
    cameraPosition.y += 1.f * window.deltaTime;
    updateView();
  });
  window.addKeyUpdateHandler(18, [&]()
  {
    cameraPosition.y -= 1.f * window.deltaTime;
    updateView();
  });
  window.addKeyPressHandler(27, [&](const auto &pressed)
  {
    if (!pressed)
      window.close();
  });
  updateView();
};

int main()
{
  GLWindow window("GLWindow", 1280, 720);
  window.clearColor = {0, 0, 0, 1};
  window.runOnThread([](auto &window)
  {
    window.setIScene(std::make_shared<TestScene>(window));
  });
  // std::this_thread::sleep_for(std::chrono::seconds(20));
  // window.close();
  window.awaitWindowThread();
};
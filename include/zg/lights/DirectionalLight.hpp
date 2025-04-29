#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
namespace zg
{
  struct Window;
}
namespace zg::lights
{
  struct DirectionalLightShadow
  {
    std::vector<size_t*> INDEX_STACK;
    IRenderer* iRenderer = 0;
    shaders::Shader *shader = 0;
    size_t directionalLightIndex = 0;
    std::shared_ptr<textures::Texture> texture;
    std::shared_ptr<textures::Framebuffer> framebuffer;
    glm::mat4 lightSpaceMatrix;
    bool lookAtSet = false;
    glm::vec3 lookAt = glm::vec3(0);
    DirectionalLightShadow(const std::vector<size_t*>& INDEX_STACK, size_t directionalLightIndex);
    DirectionalLightShadow(const DirectionalLightShadow& other);
    DirectionalLightShadow& operator=(const DirectionalLightShadow& other);
    void addShader();
    void update();
  };
}
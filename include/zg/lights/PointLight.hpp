#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
#include "../Window.hpp"
namespace zg::lights
{
  struct PointLightShadow
  {
    Window &window;
    shaders::Shader &shader;
    PointLight &pointLight;
    textures::Texture texture;
    textures::Framebuffer framebuffer;
    glm::mat4 shadowTransforms[6];
    PointLightShadow(Window &window, PointLight &pointLight);
    void updateShadowTransforms();
  };
}
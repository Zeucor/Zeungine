#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
#include "../Window.hpp"
namespace zg::lights
{
  struct DirectionalLightShadow
  {
    Window &window;
    shaders::Shader *shader = 0;
    DirectionalLight &directionalLight;
    textures::Texture texture;
    textures::Framebuffer framebuffer;
    glm::mat4 lightSpaceMatrix;
    DirectionalLightShadow(Window &window, DirectionalLight &directionalLight);
  };
}
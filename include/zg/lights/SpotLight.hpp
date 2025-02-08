#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
#include "../Window.hpp"
namespace zg::lights
{
  struct SpotLightShadow
  {
    Window &window;
    shaders::Shader *shader = 0;
    SpotLight &spotLight;
    textures::Texture texture;
    textures::Framebuffer framebuffer;
    glm::mat4 lightSpaceMatrix;
    SpotLightShadow(Window &window, SpotLight &spotLight);
  };
}
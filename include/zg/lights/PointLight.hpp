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
  struct PointLightShadow
  {
    Window &window;
    shaders::Shader *shader = 0;
    PointLight &pointLight;
    textures::Texture texture;
    textures::Framebuffer framebuffer;
    glm::mat4 shadowTransforms[6];
    PointLightShadow(Window &window, PointLight &pointLight);
    PointLightShadow& operator=(const PointLightShadow& other);
    void updateShadowTransforms();
  };
}
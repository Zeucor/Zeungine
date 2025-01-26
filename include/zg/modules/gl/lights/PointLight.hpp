#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
#include "../RenderWindow.hpp"
namespace zg::modules::gl::lights
{
	struct PointLightShadow
  {
		RenderWindow &window;
    shaders::Shader &shader;
    PointLight &pointLight;
    textures::Texture texture;
	textures::Framebuffer framebuffer;
	glm::mat4 shadowTransforms[6];
    PointLightShadow(RenderWindow &window, PointLight &pointLight);
	void updateShadowTransforms();
  };
}
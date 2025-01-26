#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
#include "../RenderWindow.hpp"
namespace zg::modules::gl::lights
{
	struct DirectionalLightShadow
  {
		RenderWindow &window;
    shaders::Shader &shader;
    DirectionalLight &directionalLight;
    textures::Texture texture;
		textures::Framebuffer framebuffer;
		glm::mat4 lightSpaceMatrix;
    DirectionalLightShadow(RenderWindow &window, DirectionalLight &directionalLight);
  };
}
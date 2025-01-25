#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
#include "../GLWindow.hpp"
namespace zg::modules::gl::lights
{
	struct DirectionalLightShadow
  {
		GLWindow &window;
    shaders::Shader &shader;
    DirectionalLight &directionalLight;
    textures::Texture texture;
		textures::Framebuffer framebuffer;
		glm::mat4 lightSpaceMatrix;
    DirectionalLightShadow(GLWindow &window, DirectionalLight &directionalLight);
  };
}
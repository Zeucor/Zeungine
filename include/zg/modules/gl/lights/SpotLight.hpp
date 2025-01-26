#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
#include "../RenderWindow.hpp"
namespace zg::modules::gl::lights
{
	struct SpotLightShadow
  {
		RenderWindow &window;
    shaders::Shader &shader;
    SpotLight &spotLight;
    textures::Texture texture;
		textures::Framebuffer framebuffer;
		glm::mat4 lightSpaceMatrix;
    SpotLightShadow(RenderWindow &window, SpotLight &spotLight);
  };
}
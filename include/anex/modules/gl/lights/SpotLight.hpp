#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
#include "../GLWindow.hpp"
namespace anex::modules::gl::lights
{
	struct SpotLightShadow
  {
		GLWindow &window;
    shaders::Shader &shader;
    SpotLight &spotLight;
    textures::Texture texture;
		textures::Framebuffer framebuffer;
		glm::mat4 lightSpaceMatrix;
    SpotLightShadow(GLWindow &window, SpotLight &spotLight);
  };
}
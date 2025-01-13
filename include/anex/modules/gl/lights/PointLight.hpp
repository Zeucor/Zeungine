#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
#include "../GLWindow.hpp"
namespace anex::modules::gl::lights
{
	struct PointLightShadow
  {
		GLWindow &window;
    shaders::Shader &shader;
    PointLight &pointLight;
    textures::Texture texture;
	textures::Framebuffer framebuffer;
	glm::mat4 shadowTransforms[6];
    PointLightShadow(GLWindow &window, PointLight &pointLight);
	void updateShadowTransforms();
  };
}
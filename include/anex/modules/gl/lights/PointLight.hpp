#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
namespace anex::modules::gl::lights
{
	struct PointLightShadow
  {
    shaders::Shader &shader;
    PointLight &pointLight;
    textures::Texture texture;
		textures::Framebuffer framebuffer;
		glm::mat4 shadowTransforms[6];
    PointLightShadow(PointLight &pointLight);
  };
}
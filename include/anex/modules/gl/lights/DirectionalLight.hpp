#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Texture.hpp"
#include "../textures/Framebuffer.hpp"
#include "./Lights.hpp"
namespace anex::modules::gl::lights
{
	struct DirectionalLightShadow
  {
    shaders::Shader &shader;
    DirectionalLight &directionalLight;
    textures::Texture texture;
		textures::Framebuffer framebuffer;
    DirectionalLightShadow(DirectionalLight &directionalLight);
  };
}
#pragma once
#include "../shaders/Shader.hpp"
#include "../textures/Framebuffer.hpp"
#include "../textures/Texture.hpp"
#include "./Lights.hpp"
namespace zg
{
	struct Window;
}
namespace zg::lights
{
	struct SpotLightShadow
	{
		Window& window;
		shaders::Shader* shader = 0;
		SpotLight& spotLight;
		std::shared_ptr<textures::Texture> texture;
		std::shared_ptr<textures::Framebuffer> framebuffer;
		glm::mat4 lightSpaceMatrix;
		SpotLightShadow(Window& window, SpotLight& spotLight);
		SpotLightShadow& operator=(const SpotLightShadow& other);
	};
} // namespace zg::lights

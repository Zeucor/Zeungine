#include <zg/lights/SpotLight.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <iostream>
using namespace zg::lights;
SpotLightShadow::SpotLightShadow(Window &window, SpotLight &spotLight):
	window(window),
	shader(*shaders::ShaderManager::getShaderByConstants(window, {"Color", "Position", "Normal", "Model", "LightSpaceMatrix"}).second),
	spotLight(spotLight),
  texture(window, glm::ivec4(4096, 4096, 1, 0), 0, textures::Texture::Depth, textures::Texture::Float),
	framebuffer(window, texture)
{
	float fov = glm::acos(glm::clamp(spotLight.outerCutoff, -1.0f, 1.0f)) * 2.0;
	glm::mat4 lightProjection = glm::perspective(fov, 1.f, spotLight.nearPlane, spotLight.farPlane);
	glm::vec3 lightDirection = glm::normalize(spotLight.direction);
	glm::vec3 lightTarget = spotLight.position + lightDirection;
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	if (glm::abs(glm::dot(worldUp, lightDirection)) > 0.99f)
	{
		worldUp = glm::vec3(1.0f, 0.0f, 0.0f); // Switch to X-axis as up
	}
	glm::vec3 right = glm::normalize(glm::cross(worldUp, lightDirection));
	glm::vec3 correctedUp = glm::normalize(glm::cross(lightDirection, right));
	glm::mat4 lightView = glm::lookAt(spotLight.position,
																	lightTarget,
																	correctedUp);
	lightSpaceMatrix = lightProjection * lightView;
};
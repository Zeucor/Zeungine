#include <zg/modules/gl/lights/DirectionalLight.hpp>
#include <zg/modules/gl/shaders/ShaderManager.hpp>
#include <iostream>
using namespace zg::modules::gl::lights;
DirectionalLightShadow::DirectionalLightShadow(RenderWindow &window, DirectionalLight &directionalLight):
	window(window),
	shader(*shaders::ShaderManager::getShaderByConstants(window, {"Color", "Position", "Normal", "Model", "LightSpaceMatrix"}).second),
	directionalLight(directionalLight),
  texture(window, glm::ivec4(4096, 4096, 1, 0), 0, textures::Texture::Depth, textures::Texture::Float),
	framebuffer(window, texture)
{
	glm::vec2 projectionDimensions = {96, 96};
	glm::mat4 lightProjection = glm::ortho(-projectionDimensions.x, projectionDimensions.x, -projectionDimensions.y, projectionDimensions.y, directionalLight.nearPlane, directionalLight.farPlane);
	glm::vec3 lightDirection = glm::normalize(directionalLight.direction);
	glm::vec3 lightTarget = directionalLight.position + lightDirection;
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	if (glm::abs(glm::dot(worldUp, lightDirection)) > 0.99f)
	{
		worldUp = glm::vec3(1.0f, 0.0f, 0.0f); // Switch to X-axis as up
	}
	glm::vec3 right = glm::normalize(glm::cross(worldUp, lightDirection));
	glm::vec3 correctedUp = glm::normalize(glm::cross(lightDirection, right));
	glm::mat4 lightView = glm::lookAt(directionalLight.position,
																	lightTarget,
																	correctedUp);
	lightSpaceMatrix = lightProjection * lightView;
};
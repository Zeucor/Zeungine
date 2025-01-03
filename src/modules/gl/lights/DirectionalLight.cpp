#include <anex/modules/gl/lights/DirectionalLight.hpp>
#include <anex/modules/gl/shaders/ShaderManager.hpp>
#include <iostream>
using namespace anex::modules::gl::lights;
DirectionalLightShadow::DirectionalLightShadow(DirectionalLight &directionalLight):
	shader(*shaders::ShaderManager::getShaderByConstants({"Color", "Position", "Normal", "Model", "LightSpaceMatrix"}).second),
	directionalLight(directionalLight),
  texture(glm::ivec4(2048, 2048, 1, 0), 0, textures::Texture::Depth, textures::Texture::Float),
	framebuffer(texture)
{
	float near_plane = 0.1f, far_plane = 4000.f;
	glm::vec2 projectionDimensions = {1024, 1024};
	glm::mat4 lightProjection = glm::ortho(-projectionDimensions.x, projectionDimensions.x, -projectionDimensions.y, projectionDimensions.y, near_plane, far_plane);
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
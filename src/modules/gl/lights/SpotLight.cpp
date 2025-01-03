#include <anex/modules/gl/lights/SpotLight.hpp>
#include <anex/modules/gl/shaders/ShaderManager.hpp>
#include <iostream>
using namespace anex::modules::gl::lights;
SpotLightShadow::SpotLightShadow(SpotLight &spotLight):
	shader(*shaders::ShaderManager::getShaderByConstants({"Color", "Position", "Normal", "Model", "LightSpaceMatrix"}).second),
	spotLight(spotLight),
  texture(glm::ivec3(4096, 4096, 1), 0, textures::Texture::Depth, textures::Texture::Float),
	framebuffer(texture)
{
	float near_plane = 1.f, far_plane = 2500.0f; // Adjust according to your spotlight's range
	float fov = glm::acos(glm::clamp(spotLight.outerCutoff, -1.0f, 1.0f)) * 2.0;
	glm::mat4 lightProjection = glm::perspective(fov, 1.f, near_plane, far_plane);
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
#include <anex/modules/gl/lights/DirectionalLight.hpp>
#include <anex/modules/gl/shaders/ShaderManager.hpp>
using namespace anex::modules::gl::lights;
DirectionalLightShadow::DirectionalLightShadow(DirectionalLight &directionalLight):
	shader(*shaders::ShaderManager::getShaderByConstants({"Position", "Model", "LightSpaceMatrix"}).second),
	directionalLight(directionalLight),
  texture(glm::ivec3(2048, 2048, 1), 0, textures::Texture::Depth, textures::Texture::Float),
	framebuffer(texture)
{
  shader.use(true);
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 10000.0f);
	glm::mat4 lightView = glm::lookAt(directionalLight.position, directionalLight.position + directionalLight.direction, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;
  shader.setBlock("LightSpaceMatrix", lightSpaceMatrix, sizeof(glm::mat4));
	shader.use(false);
};
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/shaders/ShaderManager.hpp>
using namespace anex::modules::gl;
GLEntity::GLEntity(anex::IWindow &window, const shaders::RuntimeConstants &constants, const uint32_t &indiceCount, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale):
	IEntity(window),
	VAO(constants, indiceCount),
	position(position),
	rotation(rotation),
	scale(scale),
	shader(*shaders::ShaderManager::getShaderByConstants(constants).second)
{};
void GLEntity::preRender()
{
};
void GLEntity::render()
{
  preRender();
	shader.bind();
	vaoDraw();
	shader.unbind();
};;
void GLEntity::postRender()
{
};
const glm::mat4 &GLEntity::getModelMatrix()
{
  model = glm::translate(glm::mat4(1.0f), position);
  model = glm::rotate(model, rotation.x, {1, 0, 0});
  model = glm::rotate(model, rotation.y, {0, 1, 0});
  model = glm::rotate(model, rotation.z, {0, 0, 1});
  model = glm::scale(model, scale);
  return model;
};
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/shaders/ShaderManager.hpp>
using namespace anex::modules::gl;
GLEntity::GLEntity(anex::IWindow &window, const shaders::RuntimeConstants &constants, const uint32_t &indiceCount, const uint32_t &elementCount, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale):
	IEntity(window),
	VAO((GLWindow &)window, constants, indiceCount, elementCount),
	position(position),
	rotation(rotation),
	scale(scale),
	shader(*shaders::ShaderManager::getShaderByConstants((GLWindow &)window, constants).second)
{};
void GLEntity::update()
{
	for (auto &childEntity : children)
	{
		childEntity->update();
	}
};
void GLEntity::preRender(){};
void GLEntity::render()
{
  preRender();
	shader.bind();
	vaoDraw();
	shader.unbind();
	for (auto &childEntity : children)
	{
		childEntity->render();
	}
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
	if (parentEntity)
	{
		model = parentEntity->getModelMatrix() * model;
	}
  return model;
};
void GLEntity::addChild(const std::shared_ptr<GLEntity> &child)
{
	child->parentEntity = this;
	children.push_back(child);
};
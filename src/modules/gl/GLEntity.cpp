#include <zg/modules/gl/GLEntity.hpp>
#include <zg/modules/gl/shaders/ShaderManager.hpp>
using namespace zg::modules::gl;
GLEntity::GLEntity(zg::IWindow &_window,
									 const shaders::RuntimeConstants &constants,
									 uint32_t indiceCount,
						 			 const std::vector<uint32_t> &indices,
									 uint32_t elementCount,
						 			 const std::vector<glm::vec3> &positions,
									 glm::vec3 position,
									 glm::vec3 rotation,
									 glm::vec3 scale,
									 std::string_view name):
	IEntity(_window),
	VAO(dynamic_cast<GLWindow &>(_window), constants, indiceCount, elementCount),
	indices(indices),
	positions(positions),
	position(position),
	rotation(rotation),
	scale(scale),
	shader(*shaders::ShaderManager::getShaderByConstants(dynamic_cast<GLWindow &>(_window), constants).second),
	name(name)
{};
void GLEntity::update()
{
	for (auto &childEntity : children)
	{
		childEntity.second->update();
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
		childEntity.second->render();
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
size_t GLEntity::addChild(const std::shared_ptr<GLEntity> &child)
{
	auto id = ++childrenCount;
	child->parentEntity = this;
	child->ID = id;
	children[id] = child;
	return id;
};
void GLEntity::removeChild(size_t &ID)
{
	auto iter = children.find(ID);
	if (iter == children.end())
	{
		ID = 0;
		return;
	}
	children.erase(iter);
	ID = 0;
};
// Mouse
zg::IWindow::EventIdentifier GLEntity::addMousePressHandler(const zg::IWindow::Button& button, const zg::IWindow::MousePressHandler& callback)
{
	auto& handlersPair = mousePressHandlers[button];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void GLEntity::removeMousePressHandler(const zg::IWindow::Button& button, zg::IWindow::EventIdentifier& id)
{
	auto& handlersPair = mousePressHandlers[button];
	auto handlerIter = handlersPair.second.find(id);
	if (handlerIter == handlersPair.second.end())
	{
		return;
	}
	handlersPair.second.erase(handlerIter);
	id = 0;
};
zg::IWindow::EventIdentifier GLEntity::addMouseMoveHandler(const zg::IWindow::MouseMoveHandler& callback)
{
	auto id = ++mouseMoveHandlers.first;
	mouseMoveHandlers.second[id] = callback;
	return id;
};
void GLEntity::removeMouseMoveHandler(zg::IWindow::EventIdentifier& id)
{
	auto &handlers = mouseMoveHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
zg::IWindow::EventIdentifier GLEntity::addMouseHoverHandler(const MouseHoverHandler &callback)
{
	auto id = ++mouseHoverHandlers.first;
	mouseHoverHandlers.second[id] = callback;
	return id;
};
void GLEntity::removeMouseHoverHandler(IWindow::EventIdentifier &id)
{
	auto &handlers = mouseHoverHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void GLEntity::callMousePressHandler(const zg::IWindow::Button &button, int pressed)
{
	buttons[button] = pressed;
	{
		auto handlersIter = mousePressHandlers.find(button);
		if (handlersIter == mousePressHandlers.end())
			return;
		auto& handlersMap = handlersIter->second.second;
		std::vector<zg::IWindow::MousePressHandler> handlersCopy;
		for (const auto& pair : handlersMap)
			handlersCopy.push_back(pair.second);
		for (auto& handler : handlersCopy)
		{
			handler(!!pressed);
		}
	}
};
void GLEntity::callMouseMoveHandler(glm::vec2 coords)
{
	auto& handlersMap = mouseMoveHandlers.second;
	std::vector<zg::IWindow::MouseMoveHandler> handlersCopy;
	for (const auto& pair : handlersMap)
		handlersCopy.push_back(pair.second);
	for (auto& handler : handlersCopy)
	{
		handler(coords);
	}
};
void GLEntity::callMouseHoverHandler(bool hovered)
{
	auto& handlersMap = mouseHoverHandlers.second;
	std::vector<MouseHoverHandler> handlersCopy;
	for (const auto& pair : handlersMap)
		handlersCopy.push_back(pair.second);
	for (auto& handler : handlersCopy)
	{
		handler(hovered);
	}
};
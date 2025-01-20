#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/shaders/ShaderManager.hpp>
using namespace anex::modules::gl;
GLEntity::GLEntity(anex::IWindow &window,
									 const shaders::RuntimeConstants &constants,
									 uint32_t indiceCount,
						 			 const std::vector<uint32_t> &indices,
									 uint32_t elementCount,
						 			 const std::vector<glm::vec3> &positions,
									 const glm::vec3 &position,
									 const glm::vec3 &rotation,
									 const glm::vec3 &scale,
									 std::string_view name):
	IEntity(window),
	VAO((GLWindow &)window, constants, indiceCount, elementCount),
	indices(indices),
	positions(positions),
	position(position),
	rotation(rotation),
	scale(scale),
	shader(*shaders::ShaderManager::getShaderByConstants((GLWindow &)window, constants).second),
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
anex::IWindow::EventIdentifier GLEntity::addMousePressHandler(const anex::IWindow::Button& button, const anex::IWindow::MousePressHandler& callback)
{
	auto& handlersPair = mousePressHandlers[button];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
};
void GLEntity::removeMousePressHandler(const anex::IWindow::Button& button, anex::IWindow::EventIdentifier& id)
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
anex::IWindow::EventIdentifier GLEntity::addMouseMoveHandler(const anex::IWindow::MouseMoveHandler& callback)
{
	auto id = ++mouseMoveHandlers.first;
	mouseMoveHandlers.second[id] = callback;
	return id;
};
void GLEntity::removeMouseMoveHandler(anex::IWindow::EventIdentifier& id)
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
anex::IWindow::EventIdentifier GLEntity::addMouseHoverHandler(const MouseHoverHandler &callback)
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
void GLEntity::callMousePressHandler(const anex::IWindow::Button &button, const int &pressed)
{
	buttons[button] = pressed;
	{
		auto handlersIter = mousePressHandlers.find(button);
		if (handlersIter == mousePressHandlers.end())
			return;
		auto& handlersMap = handlersIter->second.second;
		std::vector<anex::IWindow::MousePressHandler> handlersCopy;
		for (const auto& pair : handlersMap)
			handlersCopy.push_back(pair.second);
		for (auto& handler : handlersCopy)
		{
			handler(!!pressed);
		}
	}
};
void GLEntity::callMouseMoveHandler(const glm::vec2 &coords)
{
	auto& handlersMap = mouseMoveHandlers.second;
	std::vector<anex::IWindow::MouseMoveHandler> handlersCopy;
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
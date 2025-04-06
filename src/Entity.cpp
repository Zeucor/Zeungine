#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderManager.hpp>
using namespace zg;
Entity::Entity(Window& _window, Scene& _scene, const shaders::RuntimeConstants& constants, uint32_t indiceCount,
							 const std::vector<uint32_t>& _indices, uint32_t elementCount, const std::vector<glm::vec3>& _positions,
							 glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale, std::string_view _name) :
		VAO(_window, constants, indiceCount, elementCount), window(_window), scene(_scene), indices(_indices),
		positions(_positions), position(_position), rotation(_rotation), scale(_scale), name(_name)
{
	scene.entitiesByName[name] = {this, [](auto pointer) {}};
}
Entity::~Entity()
{
	auto nameIter = scene.entitiesByName.find(name);
	if (nameIter != scene.entitiesByName.end())
	{
		scene.entitiesByName.erase(nameIter);
	}
}
void Entity::update()
{
	for (auto& childEntity : children)
	{
		childEntity.second->update();
	}
}
shaders::Shader* Entity::addShader(shaders::Shader* setShader)
{
	auto data = getShaderData(window);
	auto& shader = shaders[data];
	if (shader)
		return shader;
	if (setShader)
	{
		shader = setShader;
	}
	else
	{
		shader = shaders::ShaderManager::getShaderByConstants(window, constants, data).second.get();
	}
	return shader;
}
bool Entity::isEnsured()
{
	auto data = getShaderData(window);
	return ensuredBools[data];
}
void Entity::setEnsured()
{
	auto data = getShaderData(window);
	ensuredBools[data] = true;
}
void* Entity::getShaderData(Window& window)
{
	void* data = 0;
	auto& vulkanRenderer = *dynamic_cast<VulkanRenderer*>(window.iRenderer);
	if (vulkanRenderer.currentFramebufferImpl)
	{
		data = vulkanRenderer.currentFramebufferImpl->renderPass;
	}
	else
	{
		data = vulkanRenderer.renderPass;
	}
	return data;
}
bool Entity::preRender() { return true; };
void Entity::render()
{
	auto shader = addShader();
	if (!preRender())
		return;
	shader->bind(*this);
	drawVAO();
	shader->unbind();
	for (auto& childEntity : children)
	{
		childEntity.second->render();
	}
}
void Entity::postRender() {}
const glm::mat4& Entity::getModelMatrix()
{
	model = glm::translate(glm::mat4(1.0f), position);
	model = glm::rotate(model, glm::radians(rotation.x), {1, 0, 0});
	model = glm::rotate(model, glm::radians(rotation.y), {0, 1, 0});
	model = glm::rotate(model, glm::radians(rotation.z), {0, 0, 1});
	model = glm::scale(model, scale);
	if (parentEntity)
	{
		model = parentEntity->getModelMatrix() * model;
	}
	return model;
}
size_t Entity::addChild(const std::shared_ptr<Entity>& child)
{
	auto id = ++childrenCount;
	child->parentEntity = this;
	child->ID = id;
	children[id] = child;
	return id;
}
void Entity::removeChild(size_t& ID)
{
	auto iter = children.find(ID);
	if (iter == children.end())
	{
		ID = 0;
		return;
	}
	children.erase(iter);
	ID = 0;
}
// Mouse
UniqueIdentifier Entity::addMousePressHandler(const Button& button, const MousePressHandler& callback)
{
	auto& handlersPair = mousePressHandlers[button];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
}
void Entity::removeMousePressHandler(const Button& button, UniqueIdentifier& id)
{
	auto& handlersPair = mousePressHandlers[button];
	auto handlerIter = handlersPair.second.find(id);
	if (handlerIter == handlersPair.second.end())
	{
		return;
	}
	handlersPair.second.erase(handlerIter);
	id = 0;
}
UniqueIdentifier Entity::addMouseMoveHandler(const MouseMoveHandler& callback)
{
	auto id = ++mouseMoveHandlers.first;
	mouseMoveHandlers.second[id] = callback;
	return id;
}
void Entity::removeMouseMoveHandler(UniqueIdentifier& id)
{
	auto& handlers = mouseMoveHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
}
UniqueIdentifier Entity::addMouseHoverHandler(const MouseHoverHandler& callback)
{
	auto id = ++mouseHoverHandlers.first;
	mouseHoverHandlers.second[id] = callback;
	return id;
}
void Entity::removeMouseHoverHandler(UniqueIdentifier& id)
{
	auto& handlers = mouseHoverHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
}
void Entity::callMousePressHandler(const Button& button, int pressed)
{
	buttons[button] = pressed;
	{
		auto handlersIter = mousePressHandlers.find(button);
		if (handlersIter == mousePressHandlers.end())
			return;
		auto& handlersMap = handlersIter->second.second;
		std::vector<MousePressHandler> handlersCopy;
		for (const auto& pair : handlersMap)
			handlersCopy.push_back(pair.second);
		for (auto& handler : handlersCopy)
		{
			handler(!!pressed);
		}
	}
}
void Entity::callMouseMoveHandler(glm::vec2 coords)
{
	auto& handlersMap = mouseMoveHandlers.second;
	std::vector<MouseMoveHandler> handlersCopy;
	for (const auto& pair : handlersMap)
		handlersCopy.push_back(pair.second);
	for (auto& handler : handlersCopy)
	{
		handler(coords);
	}
}
void Entity::callMouseHoverHandler(bool hovered)
{
	auto& handlersMap = mouseHoverHandlers.second;
	std::vector<MouseHoverHandler> handlersCopy;
	for (const auto& pair : handlersMap)
		handlersCopy.push_back(pair.second);
	for (auto& handler : handlersCopy)
	{
		handler(hovered);
	}
}

#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <zg/components/entities/RigidBody.hpp>
using namespace zg;
Entity::Entity(Window& _window, Scene& _scene, const shaders::RuntimeConstants& constants, uint32_t indiceCount,
							 const std::vector<uint32_t>& _indices, uint32_t elementCount, const std::vector<glm::vec3>& _positions,
							 glm::vec3 _position, glm::quat _rotation, glm::vec3 _scale, std::string_view _name) :
		VAO(_window, constants, indiceCount, elementCount), window(_window), scene(_scene), indices(_indices),
		positions(_positions), position(_position), rotation(_rotation), scale(_scale), name(_name)
{
}
Entity::~Entity()
{
	auto& components_id_index = std::get<1>(m_components).get<component_by_id>();
	for (const auto& componentEntry : components_id_index)
	{
		componentEntry.COMPONENT->onDetached();
	}
}
void Entity::preUpdate() {}
void Entity::update()
{
	preUpdate();
	for (auto& componentEntry : std::get<1>(m_components))
	{
		componentEntry.COMPONENT->onUpdate();
	}
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
glm::mat4& Entity::getModelMatrix()
{
	// Ensure model is only recomputed once per update nonce
	// Also handles the initial case where updateNonce might be 0 and scene.updateNonce is > 0
	if (updateNonce == scene.updateNonce && updateNonce != 0)
	{
		return model;
	}
	updateNonce = scene.updateNonce; // Mark as updated for this cycle

	// --- Calculate local transformation: Translate * Rotate * Scale ---

	// 1. Start with identity matrix
	glm::mat4 identity = glm::mat4(1.0f);

	// 2. Create Scale matrix
	glm::mat4 scaleMat = glm::scale(identity, scale);

	// 3. Create Rotation matrix from quaternion
	glm::mat4 rotMat = glm::mat4_cast(rotation);

	// 4. Create Translation matrix
	glm::mat4 transMat = glm::translate(identity, position);

	// 5. Combine transformations: T * R * S
	glm::mat4 localModel = transMat * rotMat * scaleMat;

	// --- Apply parent transformation ---
	if (parentEntity)
	{
		// Multiply by the parent's world matrix
		// Parent's world transform * Our local transform
		model = parentEntity->getModelMatrix() * localModel;
	}
	else
	{
		// No parent, the local model IS the world model
		model = localModel;
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
void Entity::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
	auto rigidBodyComponent = std::dynamic_pointer_cast<zg::components::entities::RigidBody>(getComponentByName("RigidBody"));
	if (rigidBodyComponent)
		rigidBodyComponent->setPosition(position);
	return;
}
void Entity::setOrientation(glm::quat newOrientation)
{
	rotation = newOrientation;
	auto rigidBodyComponent = std::dynamic_pointer_cast<zg::components::entities::RigidBody>(getComponentByName("RigidBody"));
	if (rigidBodyComponent)
		rigidBodyComponent->setOrientation(rotation);
	return;
}
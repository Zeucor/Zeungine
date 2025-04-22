#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderManager.hpp>
using namespace zg;
Entity::Entity(const EntityCreateInfo& info) :
		VAO(info.scenePointer->window.iRenderer, info.constants, info.indiceCount, info.vertexCount),
		DataStorage<Entity>(info.getDataFunctionMap, info.setDataFunctionMap, info.dataMap),
		window(info.scenePointer->window), scene(*info.scenePointer), indices(info.indices), vertices(info.vertices),
		colors(info.colors), uv2s(info.uv2s), uv3s(info.uv3s), position(info.position), rotation(info.rotation),
		scale(info.scale), children([](const auto& entity) { return entity.name; }), name(info.name),
		preUpdateFunction(info.preUpdateFunction), preRenderFunction(info.preRenderFunction),
		postRenderFunction(info.postRenderFunction)
{
	computeNormals(window.iRenderer->frontFace, indices, vertices, normals);
	updateIndices(indices);
	if (colors.size())
		updateElements("Color", colors);
	bool flipUVs = (window.iRenderer->renderer == RENDERER_VULKAN || window.iRenderer->renderer == RENDERER_METAL);
	if (uv2s.size())
	{
		flipUVsY(uv2s);
		updateElements("UV2", uv2s);
	}
	if (uv3s.size())
	{
		flipUVsY(uv3s);
		updateElements("UV3", uv3s);
	}
	updateElements("Position", vertices);
	updateElements("Normal", normals);
}
Entity::Entity(const Entity& other) :
		VAO(other.window.iRenderer, other.constants, other.indiceCount, other.vertexCount),
		DataStorage<Entity>(other.getDataFunctionMap, other.setDataFunctionMap, other.dataMap), window(other.window),
		scene(other.scene), indices(other.indices), vertices(other.vertices), colors(other.colors), uv2s(other.uv2s),
		uv3s(other.uv3s), position(other.position), rotation(other.rotation), scale(other.scale), children(other.children),
		name(other.name), preUpdateFunction(other.preUpdateFunction), preRenderFunction(other.preRenderFunction),
		postRenderFunction(other.postRenderFunction)
{
	computeNormals(window.iRenderer->frontFace, indices, vertices, normals);
	updateIndices(indices);
	if (colors.size())
		updateElements("Color", colors);
	if (uv2s.size())
		updateElements("UV2", uv2s);
	if (uv3s.size())
		updateElements("UV3", uv3s);
	updateElements("Position", vertices);
	updateElements("Normal", normals);
}
Entity::~Entity() { detachAllComponents(); }
Entity& Entity::operator=(const Entity& other) { return *this; }
void Entity::update()
{
	if (preUpdateFunction)
		preUpdateFunction(*this);
	auto componentsData = m_components.data();
	auto componentsSize = m_components.size();
	for (size_t index = 0; index < componentsSize; ++index)
		componentsData[index].onUpdate();
	auto childrenData = children.data();
	auto childrenSize = children.size();
	for (size_t index = 0; index < childrenSize; ++index)
		childrenData[index].update();
}
void Entity::render()
{
	auto shader = addShader();
	if (preRenderFunction && !preRenderFunction(*this))
		return;
	shader->bind(*this);
	const auto& model = getModelMatrix();
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, viewPointer ? viewPointer->matrix : scene.viewPointer->matrix);
	shader->setBlock("Projection", *this,
									 projectionPointer ? projectionPointer->matrix : scene.projectionPointer->matrix);
	shader->setBlock("CameraPosition", *this, viewPointer ? viewPointer->position : scene.viewPointer->position, 16);
	drawVAO();
	shader->unbind();
	auto childrenData = children.data();
	auto childrenSize = children.size();
	for (size_t index = 0; index < childrenSize; ++index)
		childrenData[index].render();
}
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
size_t Entity::addChild(const EntityCreateInfo& childCreateInfo)
{
	auto child_tuple = children.emplace_back(childCreateInfo);
	auto& childEntity = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(child_tuple);
	childEntity.ID = std::get<KEY_ID_VECTOR_ID_INDEX>(child_tuple);
	childEntity.INDEX = std::get<KEY_ID_VECTOR_INDEX_INDEX>(child_tuple);
	return childEntity.ID;
}
void Entity::removeChild(size_t& ID)
{
	auto iter = children.find_id(ID);
	if (iter == children.end())
	{
		return;
	}
	children.erase(iter);
	ID = 0;
}
// Mouse
UniqueIdentifier Entity::addMousePressHandler(const Button& button, const MousePressHandler& callback)
{
	std::lock_guard lock(handlersMutex);
	auto& handlersPair = mousePressHandlers[button];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
}
void Entity::removeMousePressHandler(const Button& button, UniqueIdentifier& id)
{
	std::lock_guard lock(handlersMutex);
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
	std::lock_guard lock(handlersMutex);
	auto id = ++mouseMoveHandlers.first;
	mouseMoveHandlers.second[id] = callback;
	return id;
}
void Entity::removeMouseMoveHandler(UniqueIdentifier& id)
{
	std::lock_guard lock(handlersMutex);
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
	std::lock_guard lock(handlersMutex);
	auto id = ++mouseHoverHandlers.first;
	mouseHoverHandlers.second[id] = callback;
	return id;
}
void Entity::removeMouseHoverHandler(UniqueIdentifier& id)
{
	std::lock_guard lock(handlersMutex);
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
		{
			std::lock_guard lock(handlersMutex);
			for (const auto& pair : handlersMap)
				handlersCopy.push_back(pair.second);
		}
		for (auto& handler : handlersCopy)
		{
			handler(!!pressed);
		}
	}
}
void Entity::callMouseMoveHandler(glm::vec2 coords)
{
	std::vector<MouseMoveHandler> handlersCopy;
	{
		auto& handlersMap = mouseMoveHandlers.second;
		std::lock_guard lock(handlersMutex);
		for (const auto& pair : handlersMap)
			handlersCopy.push_back(pair.second);
	}
	for (auto& handler : handlersCopy)
	{
		handler(coords);
	}
}
void Entity::callMouseHoverHandler(bool hovered)
{
	std::vector<MouseHoverHandler> handlersCopy;
	{
		auto& handlersMap = mouseHoverHandlers.second;
		std::lock_guard lock(handlersMutex);
		for (const auto& pair : handlersMap)
			handlersCopy.push_back(pair.second);
	}
	for (auto& handler : handlersCopy)
	{
		handler(hovered);
	}
}
void Entity::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
	// auto rigidBodyComponent =
	// std::dynamic_pointer_cast<zg::components::entities::RigidBody>(getComponentByName("RigidBody")); if
	// (rigidBodyComponent) 	rigidBodyComponent->setPosition(position);
	return;
}
void Entity::setOrientation(glm::quat newOrientation)
{
	rotation = newOrientation;
	// auto rigidBodyComponent =
	// std::dynamic_pointer_cast<zg::components::entities::RigidBody>(getComponentByName("RigidBody")); if
	// (rigidBodyComponent) 	rigidBodyComponent->setOrientation(rotation);
	return;
}

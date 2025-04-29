#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <zg/Registry.hpp>
using namespace zg;
std::unordered_map<std::string, size_t> childKeyCounts;
Entity::Entity(const EntityCreateInfo& info) :
		VAO(info.INDEX_STACK, info.constants, info.indiceCount(*this), info.vertexCount(*this)),
		DataStorage<Entity>(info.getDataFunctionMap, info.setDataFunctionMap, info.dataMap),
		INDEX_STACK(info.INDEX_STACK),
		keyedTextures(info.keyedTextures),
		position(info.position), rotation(info.rotation),
		scale(info.scale),
		indiceCount(info.indiceCount),
		indices(info.indices),
		vertexCount(info.vertexCount),
		vertices(info.vertices),
		colorCount(info.colorCount),
		colors(info.colors),
		uv2Count(info.uv2Count),
		uv2s(info.uv2s),
		uv3Count(info.uv3Count),
		uv3s(info.uv3s),
		children(
			[](const auto& entity) { return entity.name; },
			[](const std::string& key) {
				auto _key = key;
				auto keySize = _key.size(); 
				if (!keySize)
				{
					_key = std::string("Unknown");
				}
				auto childKeyCountsIter = childKeyCounts.find(_key);
				if (childKeyCountsIter == childKeyCounts.end())
				{
					childKeyCounts[_key] = 1;
					childKeyCountsIter = childKeyCounts.find(_key);
				}
				return _key + " " + std::to_string(++childKeyCountsIter->second);
			}
		),
		name(info.name),
		preUpdateFunction(info.preUpdateFunction), preRenderFunction(info.preRenderFunction),
		postRenderFunction(info.postRenderFunction),
		onAddedFunction(info.onAddedFunction),
		onRemovedFunction(info.onRemovedFunction)
{
	auto& window = Registry::getWindow(INDEX_STACK);
	auto _indices_ = indices(*this);
	auto _vertices_ = vertices(*this);
	std::vector<glm::vec3> normals;
	computeNormals(window.iRenderer->frontFace, _indices_, _vertices_, normals);
	updateIndices(_indices_);
	if (colorCount && colorCount(*this))
		updateElements("Color", colors(*this));
	bool flipUVs = (window.iRenderer->renderer == RENDERER_VULKAN || window.iRenderer->renderer == RENDERER_METAL);
	if (uv2Count && uv2Count(*this))
	{
		auto _uv2s_ = uv2s(*this);
		flipUVsY(_uv2s_);
		updateElements("UV2", _uv2s_);
	}
	if (uv3Count && uv3Count(*this))
	{
		auto _uv3s_ = uv3s(*this);
		flipUVsY(_uv3s_);
		updateElements("UV3", _uv3s_);
	}
	updateElements("Position", _vertices_);
	updateElements("Normal", normals);
}
Entity::Entity(const Entity& other) :
	VAO(other),
	ComponentHolder<Entity, components::entities::EntityComponent, components::entities::EntityComponentCreateInfo>(other),
	DataStorage<Entity>(other),
	INDEX_STACK(other.INDEX_STACK),
	keyedTextures(other.keyedTextures), position(other.position), rotation(other.rotation), scale(other.scale),
	indiceCount(other.indiceCount),
	indices(other.indices),
	vertexCount(other.vertexCount),
	vertices(other.vertices),
	colorCount(other.colorCount),
	colors(other.colors),
	uv2Count(other.uv2Count),
	uv2s(other.uv2s),
	uv3Count(other.uv3Count),
	uv3s(other.uv3s),
	children(other.children),
	name(other.name), preUpdateFunction(other.preUpdateFunction), preRenderFunction(other.preRenderFunction),
	postRenderFunction(other.postRenderFunction),
	onAddedFunction(other.onAddedFunction),
	onRemovedFunction(other.onRemovedFunction)
{}
Entity::~Entity()
{
	for (auto& child : children)
		if (child.onRemovedFunction)
			child.onRemovedFunction(child);
	children.clear();
	detachAllComponents();
}
Entity& Entity::operator=(const Entity& other)
{
	((vaos::VAO&)*this) = other;
	((ComponentHolder<Entity, components::entities::EntityComponent, components::entities::EntityComponentCreateInfo>&)*this) = other;
	((DataStorage<Entity>&)*this) = other;
	ID = other.ID;
	INDEX = other.INDEX;
	INDEX_STACK = other.INDEX_STACK;
	keyedTextures = other.keyedTextures;
	position = other.position;
	rotation = other.rotation;
	scale = other.scale;
	indiceCount = other.indiceCount;
	indices = other.indices;
	vertexCount = other.vertexCount;
	vertices = other.vertices;
	colorCount = other.colorCount;
	colors = other.colors;
	uv2Count = other.uv2Count;
	uv2s = other.uv2s;
	uv3Count = other.uv3Count;
	uv3s = other.uv3s;
	children = other.children;
	name = other.name;
	preUpdateFunction = other.preUpdateFunction;
	preRenderFunction = other.preRenderFunction;
	postRenderFunction = other.postRenderFunction;
	onAddedFunction = other.onAddedFunction;
	onRemovedFunction = other.onRemovedFunction;
	return *this;
}
void Entity::refreshVertices()
{
	auto _vertices_ = vertices(*this);
	updateElements("Position", _vertices_);
}
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
	auto& scene = Registry::getScene(INDEX_STACK);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	{
		if (viewPointer)
			viewPointer->updateMutex.lock();
		else
			scene.viewPointer->updateMutex.lock();
		shader->setBlock("View", *this, viewPointer ? viewPointer->matrix : scene.viewPointer->matrix);
		shader->setBlock("CameraPosition", *this, viewPointer ? viewPointer->position : scene.viewPointer->position, 16);
		if (viewPointer)
			viewPointer->updateMutex.unlock();
		else
			scene.viewPointer->updateMutex.unlock();
	}
	shader->setBlock("Projection", *this,
									projectionPointer ? projectionPointer->matrix : scene.projectionPointer->matrix);
	auto keyedTexturesSize = keyedTextures.size();
	auto keyedTexturesData = keyedTextures.data();
	for (size_t unit = 0; unit < keyedTexturesSize; ++unit)
		shader->setTexture(keyedTexturesData[unit].first, *this, *keyedTexturesData[unit].second, unit);
	drawVAO();
	shader->unbind();
	auto childrenData = children.data();
	auto childrenSize = children.size();
	for (size_t index = 0; index < childrenSize; ++index)
		childrenData[index].render();
}
glm::mat4& Entity::getModelMatrix()
{
	auto& scene = Registry::getScene(INDEX_STACK);
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

	model = localModel;
	// --- Apply parent transformation ---
	Entity* parentEntity = 0;
	if (Registry::getNthParentEntity(INDEX_STACK, parentEntity))
	{
		// Multiply by the parent's world matrix
		// Parent's world transform * Our local transform
		model = parentEntity->getModelMatrix() * model;
	}

	return model;
}
KeyIDVector<std::string, Entity>::EmplaceBackTuple Entity::addChild(const EntityCreateInfo& childCreateInfo)
{
	auto usingInfo = childCreateInfo;
	auto transaction = children.startTransaction();
	usingInfo.INDEX_STACK = {INDEX_STACK.begin(), INDEX_STACK.end()};
	usingInfo.INDEX_STACK.push_back(transaction.index);
	auto& childEntity = children.commitTransaction(transaction, usingInfo);
	childEntity.ID = transaction.id;
	childEntity.INDEX = transaction.index;
	(*Registry::idEntities)[childEntity.ID] = childEntity.INDEX_STACK;
	if (childEntity.onAddedFunction)
		childEntity.onAddedFunction(childEntity);
	return {transaction.key, transaction.id, transaction.index, &childEntity};
}
void Entity::removeChild(size_t& ID)
{
	auto childIter = children.find_id(ID);
	if (childIter == children.end())
	{
		return;
	}
	auto& child = *childIter;
	if (child.onRemovedFunction)
		child.onRemovedFunction(child);
	children.erase(childIter);
	auto& idEntitiesRef = *Registry::idEntities;
	auto idIter = idEntitiesRef.find(ID);
	if (idIter != idEntitiesRef.end())
	{
		idEntitiesRef.erase(idIter);
	}
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
	try
	{
		auto& rb = getComponentByName("RigidBody");
		rb.template setData<glm::vec3>("setPosition", position);
	}
	catch(...)
	{}
	return;
}
void Entity::setOrientation(glm::quat newOrientation)
{
	rotation = newOrientation;
	try
	{
		auto& rb = getComponentByName("RigidBody");
		rb.template setData<glm::quat>("setOrientation", rotation);
	}
	catch(...)
	{}
	return;
}

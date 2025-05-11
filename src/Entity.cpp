#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <zg/Registry.hpp>
#include <zg/crypto/vector.hpp>
using namespace zg;
std::unordered_map<std::string, size_t> childKeyCounts;
Entity::Entity(const EntityCreateInfo& info) :
	DataStorage<Entity>(info.getDataFunctionMap, info.setDataFunctionMap, info.dataMap),
	ID(info.ID),
	INDEX(info.INDEX),
	INDEX_STACK(info.INDEX_STACK),
	typeName(info.typeName),
	name(info.name),
	position(info.position),
	rotation(info.rotation),
	scale(info.scale),
	meshInfos(info.meshInfos),
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
	preUpdateFunction(info.preUpdateFunction), preRenderFunction(info.preRenderFunction),
	postRenderFunction(info.postRenderFunction),
	onAddedFunction(info.onAddedFunction),
	onRemovedFunction(info.onRemovedFunction),
	addToBVH(info.addToBVH)
{
	for (auto& meshInfo : info.meshInfos)
		meshIDs.push_back(Registry::addMesh(meshInfo, *this));
	for (auto& childInfo : info.childrenInfos)
		addChild(childInfo);
}
Entity::Entity(const Entity& other) :
	ComponentHolder<Entity, components::entities::EntityComponent, components::entities::EntityComponentCreateInfo>(other),
	DataStorage<Entity>(other),
	ID(other.ID),
	INDEX(other.INDEX),
	INDEX_STACK(other.INDEX_STACK),
	VALUE(other.VALUE),
	typeName(other.typeName),
	name(other.name),
	position(other.position),
	rotation(other.rotation),
	scale(other.scale),
	model(other.model),
	projectionPointer(other.projectionPointer),
	viewPointer(other.viewPointer),
	affectedByShadows(other.affectedByShadows),
	addToBVH(other.addToBVH),
	buttons(other.buttons),
	mousePressHandlers(other.mousePressHandlers),
	mouseMoveHandlers(other.mouseMoveHandlers),
	mouseHoverHandlers(other.mouseHoverHandlers),
	meshIDs(other.meshIDs),
	meshInfos(other.meshInfos),
	children(other.children),
	isTransparent(other.isTransparent),
	preUpdateFunction(other.preUpdateFunction),
	preRenderFunction(other.preRenderFunction),
	postRenderFunction(other.postRenderFunction),
	onAddedFunction(other.onAddedFunction),
	onRemovedFunction(other.onRemovedFunction)
{
	{
		std::lock_guard meshIDLock(Registry::meshIDMutex);
		for (auto& meshID : meshIDs)
			Registry::meshIDRefCounts[meshID]++;
	}
}
Entity::~Entity()
{
	for (auto& child : children)
		if (child.onRemovedFunction)
			child.onRemovedFunction(child);
	children.clear();
	for (auto& meshID : meshIDs)
		Registry::deRefMesh(meshID);
	detachAllComponents();
}
Entity& Entity::operator=(const Entity& other)
{
	((ComponentHolder<Entity, components::entities::EntityComponent, components::entities::EntityComponentCreateInfo>&)*this) = other;
	((DataStorage<Entity>&)*this) = other;
	ID = other.ID;
	INDEX = other.INDEX;
	INDEX_STACK = other.INDEX_STACK;
	VALUE = other.VALUE;
	typeName = other.typeName;
	name = other.name;
	position = other.position;
	rotation = other.rotation;
	scale = other.scale;
	model = other.model;
	projectionPointer = other.projectionPointer;
	viewPointer = other.viewPointer;
	affectedByShadows = other.affectedByShadows;
	addToBVH = other.addToBVH;
	buttons = other.buttons;
	mousePressHandlers = other.mousePressHandlers;
	mouseMoveHandlers = other.mouseMoveHandlers;
	mouseHoverHandlers = other.mouseHoverHandlers;
	{
		std::lock_guard meshIDLock(Registry::meshIDMutex);
		for (auto& meshID : meshIDs)
			Registry::meshIDRefCounts[meshID]--;
	}
	meshIDs = other.meshIDs;
	{
		std::lock_guard meshIDLock(Registry::meshIDMutex);
		for (auto& meshID : meshIDs)
			Registry::meshIDRefCounts[meshID]++;
	}
	meshInfos = other.meshInfos;
	children = other.children;
	preUpdateFunction = other.preUpdateFunction;
	preRenderFunction = other.preRenderFunction;
	postRenderFunction = other.postRenderFunction;
	onAddedFunction = other.onAddedFunction;
	onRemovedFunction = other.onRemovedFunction;
	isTransparent = other.isTransparent;
	return *this;
}
void Entity::refreshMeshes()
{
	auto meshInfosSize = meshInfos.size();
	auto meshInfosData = meshInfos.data();
	auto meshIDsSize = meshIDs.size();
	auto meshIDsData = meshIDs.data();
	for (size_t index = 0; index < meshInfosSize; ++index)
	{
		auto meshID = index < meshIDsSize ? meshIDsData[index] : 0;
		auto& meshInfo = meshInfosData[index];
		auto newSubMeshID = Registry::addMesh(meshInfo, *this);
		if (Registry::deRefMesh(meshID))
		{
			auto& scene = Registry::getScene(INDEX_STACK);
			scene.instancedDraw.removeMesh(meshID);
		}
		if (meshID != 0 && meshID != newSubMeshID)
		{
			meshIDsData[index] = newSubMeshID;
		}
		else if (meshID == 0)
		{
			meshIDs.insert(meshIDs.begin() + index, newSubMeshID);
			meshIDsSize = meshIDs.size();
			meshIDsData = meshIDs.data();
		}
	}
}
void Entity::reMeshhash()
{
	mesh_hash = zg::crypto::hashVector(meshIDs);
};
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
	if (preRenderFunction && !preRenderFunction(*this))
		return;
	for (auto& meshID : meshIDs)
	{
		auto& mesh = Registry::getMesh(meshID);
		mesh.uid = ID;
		mesh.render(*this);
	}
	// auto childrenData = children.data();
	// auto childrenSize = children.size();
	// for (size_t index = 0; index < childrenSize; ++index)
	// 	childrenData[index].render();
}
void Entity::postRender()
{
	// for (auto& meshID : meshIDs)
	// 	Registry::getMesh(meshID).setTexturesThisPass = false;
}
glm::mat4& Entity::getModelMatrix()
{
	auto& scene = Registry::getScene(INDEX_STACK);
	if (updateNonce == scene.updateNonce && updateNonce != 0)
	{
		return model;
	}
	updateNonce = scene.updateNonce;
	glm::mat4 identity = glm::mat4(1.0f);
	glm::mat4 scaleMat = glm::scale(identity, scale);
	glm::mat4 rotMat = glm::mat4_cast(rotation);
	glm::mat4 transMat = glm::translate(identity, position);
	glm::mat4 localModel = transMat * rotMat * scaleMat;
	model = localModel;
	Entity* parentEntity = 0;
	if (Registry::getNthParentEntity(INDEX_STACK, parentEntity))
	{
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
	usingInfo.ID = transaction.id;
	usingInfo.INDEX = transaction.index;
	auto& childEntity = children.commitTransaction(transaction, usingInfo);
	(*Registry::idEntities)[childEntity.ID] = childEntity.INDEX_STACK;
	if (childEntity.onAddedFunction)
		childEntity.onAddedFunction(childEntity);
	return {transaction.key, transaction.id, transaction.index, &childEntity};
}
void Entity::removeChild(size_t ID)
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
void Entity::callMousePressHandler(const Button& button, bool pressed)
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
	try
	{
		auto& rb = getComponentByName("RigidBody");
		rb.template setData<glm::quat>("setOrientation", newOrientation);
	}
	catch(...)
	{
		rotation = newOrientation;
	}
	return;
}

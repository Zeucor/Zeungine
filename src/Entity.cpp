#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <zg/Registry.hpp>
#include <zg/crypto/vector.hpp>
#include <zg/entities/SDF.hpp>
#include <zg/physics/AABB.hpp>
using namespace zg;
std::unordered_map<std::string, size_t> childKeyCounts;
Entity::Entity(const EntityCreateInfo& info) :
	DataStorage<Entity>(info.getDataFunctionMap, info.setDataFunctionMap, info.dataMap),
	ID(info.ID),
	INDEX(info.INDEX),
	INDEX_STACK(info.INDEX_STACK),
	meta_int(info.meta_int),
	meta_float(info.meta_float),
	meta_vec4(info.meta_vec4),
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
	ZGZoneScoped;
	if (onAddedFunction)
		onAddedFunction(*this);
	for (auto& meshInfo : info.meshInfos)
		meshIDs.push_back(Registry::GetSingleton().addMesh(meshInfo, *this));
	for (auto& childInfo : info.childrenInfos)
		addEntity(childInfo);
}
Entity::Entity(const Entity& other) :
	ComponentHolder<Entity, components::entities::EntityComponent, components::entities::EntityComponentCreateInfo>(other),
	DataStorage<Entity>(other),
	ID(other.ID),
	INDEX(other.INDEX),
	INDEX_STACK(other.INDEX_STACK),
	VALUE(other.VALUE),
	meta_int(other.meta_int),
	meta_float(other.meta_float),
	meta_vec4(other.meta_vec4),
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
	ZGZoneScoped;
	{
		std::lock_guard meshIDLock(Registry::GetSingleton().meshIDMutex);
		for (auto& meshID : meshIDs)
			Registry::GetSingleton().meshIDRefCounts[meshID]++;
	}
}
Entity::~Entity()
{
	ZGZoneScoped;
	for (auto& child : children)
		if (child.onRemovedFunction)
			child.onRemovedFunction(child);
	children.clear();
	for (auto& meshID : meshIDs)
		Registry::GetSingleton().deRefMesh(meshID);
}
Entity& Entity::operator=(const Entity& other)
{
	ZGZoneScoped;
	((ComponentHolder<Entity, components::entities::EntityComponent, components::entities::EntityComponentCreateInfo>&)*this) = other;
	((DataStorage<Entity>&)*this) = other;
	ID = other.ID;
	INDEX = other.INDEX;
	INDEX_STACK = other.INDEX_STACK;
	VALUE = other.VALUE;
	meta_int = other.meta_int;
	meta_float = other.meta_float;
	meta_vec4 = other.meta_vec4;
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
		std::lock_guard meshIDLock(Registry::GetSingleton().meshIDMutex);
		for (auto& meshID : meshIDs)
			Registry::GetSingleton().meshIDRefCounts[meshID]--;
	}
	meshIDs = other.meshIDs;
	{
		std::lock_guard meshIDLock(Registry::GetSingleton().meshIDMutex);
		for (auto& meshID : meshIDs)
			Registry::GetSingleton().meshIDRefCounts[meshID]++;
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
	ZGZoneScoped;
	auto meshInfosSize = meshInfos.size();
	auto meshInfosData = meshInfos.data();
	auto meshIDsSize = meshIDs.size();
	auto meshIDsData = meshIDs.data();
	for (size_t index = 0; index < meshInfosSize; ++index)
	{
		auto meshID = index < meshIDsSize ? meshIDsData[index] : 0;
		auto& meshInfo = meshInfosData[index];
		auto newSubMeshID = Registry::GetSingleton().addMesh(meshInfo, *this);
		if (Registry::GetSingleton().deRefMesh(meshID))
		{
			auto& scene = Registry::GetSingleton().getScene(INDEX_STACK);
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
Material& Entity::meshMaterial(size_t meshID)
{
	ZGZoneScoped;
	auto& mesh = Registry::GetSingleton().getMesh(meshID);
	return mesh.info.material;
}
float BoxSDF(const glm::vec3& p) {
    glm::vec3 halfExtents(0.5f, 0.5f, 0.5f);
    glm::vec3 q = glm::abs(p) - halfExtents;
    return glm::length((glm::max)(q, 0.0f)) + (glm::min)((glm::max)(q.x, q.y, q.z), 0.0f);
}
float PlaneXYSDF(const glm::vec3& p) {
    return p.z;
}
float PlaneXZSDF(const glm::vec3& p) {
    return p.y;
}
float PlaneYZSDF(const glm::vec3& p) {
    return p.x;
}
float Entity::operator()(glm::vec3 p) const
{
	// glm::vec3 p(p_cgal.x(), p_cgal.y(), p_cgal.z());
	float res = (std::numeric_limits<float>::max)();
	for (auto& meshInfo : meshInfos)
	{
        float current_sdf = (std::numeric_limits<float>::max)();
		switch (meshInfo.shapeType) {
			case ShapeType::Box:
				current_sdf = BoxSDF(p);
				break;
			case ShapeType::PlaneXY:
				current_sdf = PlaneXYSDF(p);
				break;
			case ShapeType::PlaneXZ:
				current_sdf = PlaneXZSDF(p);
				break;
			case ShapeType::PlaneYZ:
				current_sdf = PlaneYZSDF(p);
				break;
			case ShapeType::SDF:
			{
			_sdf_from_type:
				auto& sdf_rgy = SDFRegistry::GetSingleton();
				if (!meshInfo.sdf_c_function)
				{
					((MeshCreateInfo&)meshInfo).sdf_c_function = sdf_rgy.get_sdf_function(meshInfo.meta_int);
				}
				current_sdf = meshInfo.sdf_c_function(*this, p);
				break;
			};
			case ShapeType::Mesh:
			{
				if (meshInfo.meta_int > -1)
				{
					goto _sdf_from_type;
				}
				// get vertices/indices
				// Run through TriangleMeshSDF
				continue;
			}
			default: break;
		}
        res = (glm::min)(res, current_sdf);
	}
	return res;
}
glm::vec3 Entity::getBoundingSize() const
{
	physics::AABB aabb;
    std::vector<glm::vec3> points;
    for (auto& meshInfo : meshInfos)
    {
        switch (meshInfo.shapeType) {
            case ShapeType::PlaneXY:
            case ShapeType::PlaneXZ:
            case ShapeType::PlaneYZ:
            case ShapeType::SDF:
            case ShapeType::Box:
            {
			_box_sphere:
                float half_extent = 0.5f;
                points.push_back(glm::vec3( half_extent,  half_extent,  half_extent) * scale);
                points.push_back(glm::vec3(-half_extent,  half_extent,  half_extent) * scale);
                points.push_back(glm::vec3( half_extent, -half_extent,  half_extent) * scale);
                points.push_back(glm::vec3( half_extent,  half_extent, -half_extent) * scale);
                points.push_back(glm::vec3(-half_extent, -half_extent,  half_extent) * scale);
                points.push_back(glm::vec3(-half_extent,  half_extent, -half_extent) * scale);
                points.push_back(glm::vec3( half_extent, -half_extent, -half_extent) * scale);
                points.push_back(glm::vec3(-half_extent, -half_extent, -half_extent) * scale);
                break;
            }
            case ShapeType::Mesh:
            {
				if (meshInfo.meta_int > -1)
					goto _box_sphere;
				auto minfo = meshInfo.info(*this);
				auto& vertices = minfo.vertices;
				points.reserve(vertices.size());
				for (auto& v : vertices)
					points.push_back(glm::vec3(v.x, v.y, v.z) * scale);
                break;
            }
            default:
                break;
        }
    }
	for (auto& p : points)
	{
		aabb.encompass(p);
	}
	return aabb._max - aabb._min;
}
void Entity::reMeshhash()
{
	ZGZoneScoped;
	mesh_hash = zg::crypto::hashVector(meshIDs);
};
void Entity::update()
{
	ZGZoneScoped;
	auto childrenData = children.data();
	auto childrenSize = children.size();
	for (size_t index = 0; index < childrenSize; ++index)
		childrenData[index].update();
	if (preUpdateFunction)
		preUpdateFunction(*this);
	auto componentsData = m_components.data();
	auto componentsSize = m_components.size();
	for (size_t index = 0; index < componentsSize; ++index)
		componentsData[index].onUpdate();
}
void Entity::render()
{
	ZGZoneScoped;
	auto childrenData = children.data();
	auto childrenSize = children.size();
	for (size_t index = 0; index < childrenSize; ++index)
		childrenData[index].render();
	if (preRenderFunction && !preRenderFunction(*this))
		return;
	if (!skipRender)
	{
		for (auto& meshID : meshIDs)
		{
			auto& mesh = Registry::GetSingleton().getMesh(meshID);
			mesh.render(*this);
		}
	}
}
size_t Entity::getOpaqueChildDrawCount()
{
	auto& rgy = Registry::GetSingleton();
	size_t c = 0;
	auto childrenSize = children.size();
	auto childrenData = children.data();
	std::function<void(Entity&)> countEntity;
	countEntity = [&](auto& entity)
	{
		if (!entity.isTransparent && !entity.skipRender && ((!entity.renderedThisPass && entity.renderOncePerPass) || !entity.renderOncePerPass))
		{
			for (auto& meshID : entity.meshIDs)
			{
				auto& mesh = rgy.getMesh(meshID);
				bool meshIsTransparent = false;
				for (auto& keyedPair : mesh.info.keyedTextures)
				{
					if (keyedPair.second->isTransparent)
					{
						meshIsTransparent = true;
						break;
					}
				}
				if (!meshIsTransparent)
				{
					c++;
				}
			}
		}
		for (auto& child : entity.children)
			countEntity(child);
	};
	for (size_t i = 0; i < childrenSize; ++i)
	{
		auto& child = childrenData[i];
		countEntity(child);
	}
	return c;
}
std::vector<std::pair<Entity*, Mesh*>> Entity::getOpaqueChildDrawList()
{
	auto& rgy = Registry::GetSingleton();
	auto size = getOpaqueChildDrawCount();
	std::vector<std::pair<Entity*, Mesh*>> drawList;
	drawList.resize(size);
	auto drawListData = drawList.data();
	std::function<void(Entity&)> addEntity;
	size_t index = 0;
	addEntity = [&](auto& entity)
	{
		if (!entity.isTransparent && !entity.skipRender && ((!entity.renderedThisPass && entity.renderOncePerPass) || !entity.renderOncePerPass))
		{
			for (auto& meshID : entity.meshIDs)
			{
				auto& mesh = rgy.getMesh(meshID);
				bool meshIsTransparent = false;
				for (auto& keyedPair : mesh.info.keyedTextures)
				{
					if (keyedPair.second->isTransparent)
					{
						meshIsTransparent = true;
						break;
					}
				}
				if (!meshIsTransparent)
				{
					drawListData[index++] = {&entity, &mesh};
					entity.renderedThisPass = true;
				}
			}
		}
		for (auto& child : entity.children)
			addEntity(child);
	};
	auto childrenSize = children.size();
	auto childrenData = children.data();
	for (size_t i = 0; i < childrenSize; ++i)
	{
		auto& child = childrenData[i];
		addEntity(child);
	}
	return drawList;
}
size_t Entity::getTransparentChildDrawCount()
{
	auto& rgy = Registry::GetSingleton();
	size_t c = 0;
	auto childrenSize = children.size();
	auto childrenData = children.data();
	std::function<void(Entity&)> countEntity;
	countEntity = [&](auto& entity)
	{
		if (!entity.skipRender && ((!entity.renderedThisPass && entity.renderOncePerPass) || !entity.renderOncePerPass))
		{
			for (auto& meshID : entity.meshIDs)
			{
				auto& mesh = rgy.getMesh(meshID);
				bool meshIsTransparent = false;
				for (auto& keyedPair : mesh.info.keyedTextures)
				{
					if (keyedPair.second->isTransparent)
					{
						meshIsTransparent = true;
						break;
					}
				}
				if (meshIsTransparent || entity.isTransparent)
				{
					c++;
				}
			}
		}
		for (auto& child : entity.children)
			countEntity(child);
	};
	for (size_t i = 0; i < childrenSize; ++i)
	{
		auto& child = childrenData[i];
		countEntity(child);
	}
	return c;
}
std::vector<std::pair<Entity*, Mesh*>> Entity::getTransparentChildDrawList()
{
	auto& rgy = Registry::GetSingleton();
	auto size = getTransparentChildDrawCount();
	std::vector<std::pair<Entity*, Mesh*>> drawList;
	drawList.resize(size);
	auto drawListData = drawList.data();
	std::function<void(Entity&)> addEntity;
	size_t index = 0;
	addEntity = [&](auto& entity)
	{
		if (!entity.skipRender && ((!entity.renderedThisPass && entity.renderOncePerPass) || !entity.renderOncePerPass))
		{
			for (auto& meshID : entity.meshIDs)
			{
				auto& mesh = rgy.getMesh(meshID);
				bool meshIsTransparent = false;
				for (auto& keyedPair : mesh.info.keyedTextures)
				{
					if (keyedPair.second->isTransparent)
					{
						meshIsTransparent = true;
						break;
					}
				}
				if (meshIsTransparent || entity.isTransparent)
				{
					drawListData[index++] = {&entity, &mesh};
					entity.renderedThisPass = true;
				}
			}
		}
		for (auto& child : entity.children)
			addEntity(child);
	};
	auto childrenSize = children.size();
	auto childrenData = children.data();
	for (size_t i = 0; i < childrenSize; ++i)
	{
		auto& child = childrenData[i];
		addEntity(child);
	}
	if (drawList.size())
	{
		auto& scene = rgy.getScene(drawList.front().first->INDEX_STACK);
		ZGZoneScoped;
		std::sort(drawList.begin(), drawList.end(), [&](auto& a, auto& b)
		{
			auto& ACameraPosition = a.first->viewPointer ? a.first->viewPointer->position : scene.viewPointer->position;
			auto& BCameraPosition = b.first->viewPointer ? b.first->viewPointer->position : scene.viewPointer->position;
			auto A_model_tuple = a.first->decomposeModel();
			auto B_model_tuple = b.first->decomposeModel();
			auto distA = glm::distance(std::get<0>(A_model_tuple), ACameraPosition);
			auto distB = glm::distance(std::get<0>(B_model_tuple), BCameraPosition);
			return distA > distB;
		});
	}
	return drawList;
}
void Entity::postRender()
{
	ZGZoneScoped;
	renderedThisPass = false;
	auto childrenSize = children.size();
	auto childrenData = children.data();
	for (size_t i = 0; i < childrenSize; ++i)
		childrenData[i].postRender();
}
glm::mat4& Entity::getModelMatrix()
{
	ZGZoneScoped;
	auto& scene = Registry::GetSingleton().getScene(INDEX_STACK);
	if (updateTime == scene.updateTime && updateTime != 0)
	{
		return model;
	}
	updateTime = scene.updateTime;
	glm::mat4 identity = glm::mat4(1.0f);
	glm::mat4 scaleMat = glm::scale(identity, scale);
	glm::mat4 rotMat = glm::mat4_cast(rotation);
	glm::mat4 transMat = glm::translate(identity, position);
	glm::mat4 localModel = transMat * rotMat * scaleMat;
    model = localModel;
    Entity* parentEntity = nullptr;
    if (Registry::GetSingleton().getNthParentEntity(INDEX_STACK, parentEntity) && parentEntity != nullptr)
    {
		auto parent_model_tuple = parentEntity->decomposeModel();
        glm::mat4 parentIdentity = glm::mat4(1.0f);
        glm::mat4 parentRotMat = glm::mat4_cast(std::get<1>(parent_model_tuple));
        glm::mat4 parentTransMat = glm::translate(parentIdentity, std::get<0>(parent_model_tuple));
        glm::mat4 parentTransformWithoutScale = parentTransMat * parentRotMat;
        model = parentTransformWithoutScale * model;
    }
    return model;
}
std::tuple<glm::vec3, glm::quat, glm::vec3, glm::vec3, glm::vec4> Entity::decomposeModel()
{
	std::tuple<glm::vec3, glm::quat, glm::vec3, glm::vec3, glm::vec4> tuple;
	glm::decompose(getModelMatrix(), std::get<2>(tuple), std::get<1>(tuple), std::get<0>(tuple), std::get<3>(tuple), std::get<4>(tuple));
	return tuple;
}
glm::vec3 Entity::getModelPosition()
{
	return std::get<0>(decomposeModel());
}
glm::quat Entity::getModelRotation()
{
	return std::get<1>(decomposeModel());
}
glm::vec3 Entity::getModelScale()
{
	return std::get<2>(decomposeModel());
}
KeyIDVector<std::string, Entity>::EmplaceBackTuple Entity::addEntity(const EntityCreateInfo& childCreateInfo)
{
	ZGZoneScoped;
	auto usingInfo = childCreateInfo;
	auto transaction = children.startTransaction();
	usingInfo.INDEX_STACK = {INDEX_STACK.begin(), INDEX_STACK.end()};
	usingInfo.INDEX_STACK.push_back(transaction.index);
	usingInfo.ID = transaction.id;
	usingInfo.INDEX = transaction.index;
	auto& childEntity = children.commitTransaction(transaction, usingInfo);
	Registry::GetSingleton().idEntities[childEntity.ID] = childEntity.INDEX_STACK;
	if (childEntity.onAddedFunction)
		childEntity.onAddedFunction(childEntity);
	return {transaction.key, transaction.id, transaction.index, &childEntity};
}
void Entity::removeEntity(size_t ID)
{
	ZGZoneScoped;
	auto childIter = children.find_id(ID);
	if (childIter == children.end())
	{
		return;
	}
	auto& child = *childIter;
	if (child.onRemovedFunction)
		child.onRemovedFunction(child);
	child.detachAllComponents();
	children.erase(childIter);
	auto& idEntities = Registry::GetSingleton().idEntities;
	auto idIter = idEntities.find(ID);
	if (idIter != idEntities.end())
	{
		idEntities.erase(idIter);
	}
}
// Mouse
UniqueIdentifier Entity::addMousePressHandler(const Button& button, const MousePressHandler& callback)
{
	ZGZoneScoped;
	std::lock_guard lock(handlersMutex);
	auto& handlersPair = mousePressHandlers[button];
	auto id = ++handlersPair.first;
	handlersPair.second[id] = callback;
	return id;
}
void Entity::removeMousePressHandler(const Button& button, UniqueIdentifier& id)
{
	ZGZoneScoped;
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
	ZGZoneScoped;
	std::lock_guard lock(handlersMutex);
	auto id = ++mouseMoveHandlers.first;
	mouseMoveHandlers.second[id] = callback;
	return id;
}
void Entity::removeMouseMoveHandler(UniqueIdentifier& id)
{
	ZGZoneScoped;
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
	ZGZoneScoped;
	std::lock_guard lock(handlersMutex);
	auto id = ++mouseHoverHandlers.first;
	mouseHoverHandlers.second[id] = callback;
	return id;
}
void Entity::removeMouseHoverHandler(UniqueIdentifier& id)
{
	ZGZoneScoped;
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
	ZGZoneScoped;
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
	ZGZoneScoped;
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
	ZGZoneScoped;
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
	ZGZoneScoped;
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
	ZGZoneScoped;
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

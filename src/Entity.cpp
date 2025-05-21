#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <zg/Registry.hpp>
#include <zg/crypto/vector.hpp>
#include <zg/entities/SDF.hpp>
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
	for (auto& meshInfo : info.meshInfos)
		meshIDs.push_back(Registry::GetSingleton().addMesh(meshInfo, *this));
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
	detachAllComponents();
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
K::FT Entity::operator()(K::Point_3 p_cgal) const
{
	glm::vec3 p(p_cgal.x(), p_cgal.y(), p_cgal.z());
	float res = 0.0;
	for (auto& meshInfo : meshInfos)
	{
        float current_sdf = (std::numeric_limits<float>::max)();
		switch (meshInfo.shapeType) {
			case ShapeType::Box:
				current_sdf = BoxSDF(p);
				break;
			case ShapeType::Plane:
				current_sdf = PlaneXYSDF(p);
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
K::Sphere_3 Entity::get_suggested_bounding_sphere(float multiplier) const
{
    std::vector<K::Point_3> points;

    for (auto& meshInfo : meshInfos)
    {
        switch (meshInfo.shapeType) {
            case ShapeType::Plane:
            case ShapeType::SDF:
            case ShapeType::Box:
            {
			_box_sphere:
                float half_extent = 0.5f;
                points.push_back(K::Point_3( half_extent,  half_extent,  half_extent));
                points.push_back(K::Point_3(-half_extent,  half_extent,  half_extent));
                points.push_back(K::Point_3( half_extent, -half_extent,  half_extent));
                points.push_back(K::Point_3( half_extent,  half_extent, -half_extent));
                points.push_back(K::Point_3(-half_extent, -half_extent,  half_extent));
                points.push_back(K::Point_3(-half_extent,  half_extent, -half_extent));
                points.push_back(K::Point_3( half_extent, -half_extent, -half_extent));
                points.push_back(K::Point_3(-half_extent, -half_extent, -half_extent));
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
					points.push_back(K::Point_3(v.x, v.y, v.z));
                break;
            }
            default:
                break;
        }
    }

    if (points.empty()) {
        return K::Sphere_3(K::Point_3(0,0,0), 0.0);
    }

    CGAL::Min_sphere_d<CGAL::Min_sphere_annulus_d_traits_3<K>> min_sphere(points.begin(), points.end());

    K::Point_3 center = min_sphere.center();
    K::FT squared_radius = min_sphere.squared_radius();

    double radius_double = std::sqrt(CGAL::to_double(squared_radius));
    double scaled_radius_double = radius_double * multiplier;
    if (scaled_radius_double < 0.0) scaled_radius_double = 0.0;

    K::FT scaled_squared_radius = K::FT(scaled_radius_double) * K::FT(scaled_radius_double);
    
    return K::Sphere_3(center, scaled_squared_radius);
}
void Entity::reMeshhash()
{
	ZGZoneScoped;
	mesh_hash = zg::crypto::hashVector(meshIDs);
};
void Entity::update()
{
	ZGZoneScoped;
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
	ZGZoneScoped;
	if (preRenderFunction && !preRenderFunction(*this))
		return;
	for (auto& meshID : meshIDs)
	{
		auto& mesh = Registry::GetSingleton().getMesh(meshID);
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
	ZGZoneScoped;
	// for (auto& meshID : meshIDs)
	// 	Registry::GetSingleton().getMesh(meshID).setTexturesThisPass = false;
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
	Entity* parentEntity = 0;
	if (Registry::GetSingleton().getNthParentEntity(INDEX_STACK, parentEntity))
	{
		model = parentEntity->getModelMatrix() * model;
	}
	return model;
}
KeyIDVector<std::string, Entity>::EmplaceBackTuple Entity::addChild(const EntityCreateInfo& childCreateInfo)
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
void Entity::removeChild(size_t ID)
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

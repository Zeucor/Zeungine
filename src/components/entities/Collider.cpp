#include <zg/Entity.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/physics/AABB.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
using namespace zg::components::entities;
using zg::physics::AABB;
BoxShapeData::BoxShapeData(glm::vec3 halfExtents) : halfExtents(halfExtents) {}
glm::mat3 BoxShapeData::calculateInverseInertiaBody(float mass)
{
	// Calculate dimensions from half extents
	float width = 2.0f * halfExtents.x;
	float height = 2.0f * halfExtents.y;
	float depth = 2.0f * halfExtents.z;

	// Calculate the inertia tensor components
	float Ixx = (1.0f / 12.0f) * mass * (height * height + depth * depth);
	float Iyy = (1.0f / 12.0f) * mass * (width * width + depth * depth);
	float Izz = (1.0f / 12.0f) * mass * (width * width + height * height);

	// Construct the inertia tensor matrix
	glm::mat3 inertiaTensor = glm::mat3(0.0f); // Initialize to zero
	inertiaTensor[0][0] = Ixx;
	inertiaTensor[1][1] = Iyy;
	inertiaTensor[2][2] = Izz;

	glm::mat3 inverseInertiaTensor = glm::inverse(inertiaTensor);
	return inverseInertiaTensor;
}
MeshShapeData::MeshShapeData(Entity& entity) : entity(entity) {}
glm::mat3 MeshShapeData::calculateInverseInertiaBody(float mass)
{
	glm::mat3 inverseInertiaBody(0.0f);
	return inverseInertiaBody;
}
Collider::Collider(const ColliderInfo& info) :
		IEntityComponent("Collider"), info(info), transform(&info.entity.getModelMatrix()),
		deltaTime(info.entity.window.deltaTime)
{
}
void Collider::onAttached()
{
	ownerRigidBody = dynamic_cast<RigidBody*>(info.entity.getComponentByName("RigidBody").get());
	if (!ownerRigidBody)
	{
		throw std::runtime_error("Collider Entity[" + std::to_string(info.entity.ID) + ", " + info.entity.name +
														 "] has no RigidBody component. You must add one before adding a Collider");
	}
	ownerRigidBody->addCollider(this);
	updateWorldAABB();
	std::cout << "Collider attached." << std::endl;
}
void Collider::onUpdate()
{
	if (ownerRigidBody && !ownerRigidBody->isStatic())
	{
		updateWorldAABB();
	}
}
void Collider::onDetached()
{
	if (!ownerRigidBody)
	{
		throw std::runtime_error("Collider Entity[" + std::to_string(info.entity.ID) + ", " + info.entity.name +
														 "] has no RigidBody component during detachment. This should not happen");
	}
	else
	{
		// Unregister from the owning RigidBody
		ownerRigidBody->removeCollider(this);
	}
	ownerRigidBody = nullptr;
	transform = nullptr;
	std::cout << "Collider detached." << std::endl;
}
void Collider::updateWorldAABB()
{
	if (!ownerRigidBody || !ownerRigidBody->transform)
	{
		worldAABB._min = worldAABB._max = glm::vec3(0.0f);
		return;
	}

	// Combine world transform with collider's local offset and rotation
	glm::mat4 localOffsetTransform = glm::translate(glm::mat4(1.0f), info.offset) * glm::mat4_cast(info.rotationOffset);
	glm::mat4 finalTransform = (*ownerRigidBody->transform) * localOffsetTransform;
	auto shapeType = info.shapeData ? info.shapeData->getType() : ShapeType::_Count;
	auto shape = info.shapeData ? info.shapeData.get() : (ShapeData*)0;
	auto& body = *ownerRigidBody;

	if (shapeType == ShapeType::Box)
	{
		const auto* boxData = static_cast<const BoxShapeData*>(shape);
		if (!boxData)
		{
			worldAABB._min = worldAABB._max = body.getPosition();
			return;
		}
		const glm::vec3 h = boxData->halfExtents;
		glm::vec3 localVertices[8] = {/* ... box vertices ... */
																	{-h.x, -h.y, -h.z}, {h.x, -h.y, -h.z}, {h.x, h.y, -h.z}, {-h.x, h.y, -h.z},
																	{-h.x, -h.y, h.z},	{h.x, -h.y, h.z},	 {h.x, h.y, h.z},	 {-h.x, h.y, h.z}};
		glm::vec3 firstWorldVertex = glm::vec3(finalTransform * glm::vec4(localVertices[0], 1.0f));
		worldAABB._min = worldAABB._max = firstWorldVertex;
		for (int i = 1; i < 8; ++i)
		{
			glm::vec3 worldVertex = glm::vec3(finalTransform * glm::vec4(localVertices[i], 1.0f));
			worldAABB._min = (glm::min)(worldAABB._min, worldVertex);
			worldAABB._max = (glm::max)(worldAABB._max, worldVertex);
		}
	}
	else if (shapeType == ShapeType::Mesh)
	{
		const auto* meshData = static_cast<const MeshShapeData*>(shape);
		auto& entity = meshData->entity;
		auto& vertices = entity.positions;
		auto& indices = entity.indices;
		if (!meshData || vertices.empty())
		{
			worldAABB._min = worldAABB._max = body.getPosition();
			return;
		}
		// Calculate AABB encompassing all transformed mesh vertices
		glm::vec3 firstWorldVertex = glm::vec3(finalTransform * glm::vec4(vertices[0], 1.0f));
		worldAABB._min = worldAABB._max = firstWorldVertex;
		for (size_t i = 1; i < vertices.size(); ++i)
		{
			glm::vec3 worldVertex = glm::vec3(finalTransform * glm::vec4(vertices[i], 1.0f));
			worldAABB._min = (glm::min)(worldAABB._min, worldVertex);
			worldAABB._max = (glm::max)(worldAABB._max, worldVertex);
		}
	}
	else
	{
		// Default for other shapes (e.g., sphere) or fallback
		worldAABB._min = worldAABB._max = body.getPosition(); // Placeholder
	}
}
/**
 * @brief Calculates the world-space Axis-Aligned Bounding Box (AABB)
 * that encloses the collider's volume swept over a given time interval.
 * @details This function predicts the collider's position and orientation at the
 * end of the time interval 'dt' based on its owner RigidBody's velocity.
 * It then computes the AABB at the start (t=0) and the predicted end (t=dt)
 * and merges them to create a conservative bounding box covering the motion.
 * This is crucial for CCD broadphase algorithms.
 * @param dt The time interval (delta time) over which to calculate the swept volume.
 * @return AABB The world-space AABB encompassing the collider's motion during dt.
 */
AABB Collider::getSweptWorldAABB(float dt) const
{
	auto body = ownerRigidBody;
	if (!body)
	{
		return getWorldAABB();
	}
	AABB startAABB = getWorldAABB();
	if (body->isStatic())
	{
		return startAABB;
	}
	auto finalTransforms = scenes::PhysicsScene::getTransformsAtTime(body, dt);
	AABB endAABB = calculateWorldAABB(finalTransforms.first, finalTransforms.second);
	AABB sweptAABB = AABB::merge(startAABB, endAABB);
	return sweptAABB;
}

/**
 * @brief Calculates the world-space AABB for the collider given a specific world transform matrix.
 * @details This helper function takes a pre-calculated world transform matrix (which includes
 * body transform + collider offset/rotation) and computes the AABB based on the
 * collider's shape type and data.
 * @param worldTransform The specific world transform matrix to use for calculation.
 * @return AABB The calculated world-space AABB.
 */
AABB Collider::calculateWorldAABB(const glm::vec3& center, const glm::quat& rotation) const
{
	auto localAABB = info.shapeData->getLocalAABB();
	std::vector<glm::vec3> localCorners(8);
	localCorners[0] = localAABB._min;
	localCorners[1] = glm::vec3(localAABB._max.x, localAABB._min.y, localAABB._min.z);
	localCorners[2] = glm::vec3(localAABB._max.x, localAABB._max.y, localAABB._min.z);
	localCorners[3] = glm::vec3(localAABB._min.x, localAABB._max.y, localAABB._min.z);
	localCorners[4] = glm::vec3(localAABB._min.x, localAABB._min.y, localAABB._max.z);
	localCorners[5] = glm::vec3(localAABB._max.x, localAABB._min.y, localAABB._max.z);
	localCorners[6] = localAABB._max;
	localCorners[7] = glm::vec3(localAABB._min.x, localAABB._max.y, localAABB._max.z);
	AABB _worldAABB_;
	_worldAABB_._min = glm::vec3((std::numeric_limits<float>::max)());
	_worldAABB_._max = glm::vec3((std::numeric_limits<float>::lowest)());
	for (const auto& localCorner : localCorners)
	{
		glm::vec3 worldCorner = center + (rotation * localCorner);
		_worldAABB_.encompass(worldCorner);
	}
	return _worldAABB_;
}

// Assume getWorldAABB() calls calculateWorldAABB with the *current* transform
const AABB& Collider::getWorldAABB() const
{
	return (((AABB&)worldAABB) = calculateWorldAABB(ownerRigidBody->getPosition(), ownerRigidBody->getOrientation()));
}
ShapeType Collider::getShapeType() const { return info.shapeData ? info.shapeData->getType() : ShapeType::_Count; }
PhysicsMaterial& Collider::getPhysicsMaterial() { return info.material; }
const PhysicsMaterial& Collider::getPhysicsMaterial() const { return info.material; }
glm::vec3& Collider::getOffset() { return info.offset; }
const glm::vec3& Collider::getOffset() const { return info.offset; }
glm::quat& Collider::getRotationOffset() { return info.rotationOffset; }
const glm::quat& Collider::getRotationOffset() const { return info.rotationOffset; }
bool Collider::getIsSensor() { return info.isSensor; }
ColliderInfo& Collider::getColliderInfo() { return info; }
RigidBody* Collider::getOwnerRigidBody() { return ownerRigidBody; }
const RigidBody* Collider::getOwnerRigidBody() const { return ownerRigidBody; }
const glm::mat4* Collider::getTransform() { return transform; }
glm::vec3 Collider::getCenterAtTime(float t) const
{
	// Example implementation:
	AABB initialAABB = getWorldAABB();
	glm::vec3 initialCenter = initialAABB.getCenter();
	if (ownerRigidBody)
	{
		return initialCenter + ownerRigidBody->linearVelocity * t;
	}
	return initialCenter; // Static object
}

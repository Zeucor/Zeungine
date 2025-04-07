#include <zg/Entity.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
using namespace zg::components::entities;
BoxShapeData::BoxShapeData(glm::vec3 halfExtents) : halfExtents(halfExtents) {};
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
		// Default AABB or handle error
		worldAABB._min = glm::vec3(0.0f);
		worldAABB._max = glm::vec3(0.0f);
		return;
	}

	const auto* boxData = static_cast<const BoxShapeData*>(info.shapeData.get());
	if (!boxData || info.shapeData->getType() != ShapeType::Box)
	{ // Check shape type too
		// Handle non-box or missing data - maybe calculate based on owner bounds?
		// For now, set to a point or default
		worldAABB._min = ownerRigidBody->getPosition(); // Approx center
		worldAABB._max = ownerRigidBody->getPosition();
		return;
	}

	// Combine world transform with collider's local offset and rotation
	glm::mat4 localOffsetTransform = glm::translate(glm::mat4(1.0f), info.offset) * glm::mat4_cast(info.rotationOffset);
	glm::mat4 finalTransform = (*ownerRigidBody->transform) * localOffsetTransform;

	// Get box half extents
	const glm::vec3 h = boxData->halfExtents;

	// Define the 8 local vertices of the box
	glm::vec3 localVertices[8] = {{-h.x, -h.y, -h.z}, {h.x, -h.y, -h.z}, {h.x, h.y, -h.z}, {-h.x, h.y, -h.z},
																{-h.x, -h.y, h.z},	{h.x, -h.y, h.z},	 {h.x, h.y, h.z},	 {-h.x, h.y, h.z}};

	// Initialize min/max with the first transformed vertex
	glm::vec3 firstWorldVertex = glm::vec3(finalTransform * glm::vec4(localVertices[0], 1.0f));
	worldAABB._min = firstWorldVertex;
	worldAABB._max = firstWorldVertex;

	// Transform remaining vertices and update min/max
	for (int i = 1; i < 8; ++i)
	{
		glm::vec3 worldVertex = glm::vec3(finalTransform * glm::vec4(localVertices[i], 1.0f));
		worldAABB._min = (glm::min)(worldAABB._min, worldVertex);
		worldAABB._max = (glm::max)(worldAABB._max, worldVertex);
	}
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
const glm::mat4* Collider::getTransform() { return transform; }
zg::physics::AABB& Collider::getWorldAABB() { return worldAABB; }
const zg::physics::AABB& Collider::getWorldAABB() const { return worldAABB; }

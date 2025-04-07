#include <zg/Entity.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
using namespace zg::components::entities;
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

	// Ensure we have necessary data
	if (!transform || !info.shapeData)
		return;

	// Get shape type
	ShapeType type = info.shapeData->getType();

	// Reset AABB before calculation
	worldAABB.reset();

	// --- Calculate AABB based on Shape Type ---
	if (type == ShapeType::Box)
	{
		const auto* boxData = static_cast<const BoxShapeData*>(info.shapeData.get());
		const glm::vec3 h = boxData->halfExtents; // Local half-extents

		// Define the 8 local corners of the box relative to the collider's offset
		glm::vec3 localCorners[8] = {info.offset + glm::vec3(-h.x, -h.y, -h.z), info.offset + glm::vec3(h.x, -h.y, -h.z),
																 info.offset + glm::vec3(h.x, h.y, -h.z),		info.offset + glm::vec3(-h.x, h.y, -h.z),
																 info.offset + glm::vec3(-h.x, -h.y, h.z),	info.offset + glm::vec3(h.x, -h.y, h.z),
																 info.offset + glm::vec3(h.x, h.y, h.z),		info.offset + glm::vec3(-h.x, h.y, h.z)};

		// TODO: Apply local collider rotation offset (info.rotationOffset) to localCorners *before* world transform if
		// needed. glm::mat4 localRotMat = glm::mat4_cast(info.rotationOffset); // If rotationOffset is used

		// Transform corners to world space and find min/max
		for (int i = 0; i < 8; ++i)
		{
			// Apply local rotation if necessary: rotatedCorner = localRotMat * glm::vec4(localCorners[i], 1.0f);
			glm::vec4 worldCorner = (*transform) * glm::vec4(localCorners[i], 1.0f);
			worldAABB.encompass(glm::vec3(worldCorner));
		}
	}
	else if (type == ShapeType::Sphere)
	{
		const auto* sphereData = static_cast<const SphereShapeData*>(info.shapeData.get());
		float radius = sphereData->radius;

		// Calculate world center of the sphere (considering offset)
		// TODO: Apply local rotation offset if needed
		glm::vec3 localCenter = info.offset;
		glm::vec3 worldCenter = glm::vec3((*transform) * glm::vec4(localCenter, 1.0f));

		// Account for entity scale - find the maximum scale component
		// Note: This assumes uniform scaling for simplicity. Non-uniform scaling makes sphere AABBs tricky.
		glm::vec3 scale =
			glm::vec3(glm::length((*transform)[0]), glm::length((*transform)[1]), glm::length((*transform)[2]));
		float maxScale = (std::max)({scale.x, scale.y, scale.z});
		float worldRadius = radius * maxScale;

		worldAABB._min = worldCenter - glm::vec3(worldRadius);
		worldAABB._max = worldCenter + glm::vec3(worldRadius);
	}
	// TODO: Implement AABB calculation for other shapes (Capsule, Mesh, ConvexHull)
	else
	{
		// Default: Use a small box around the entity's origin as a fallback
		glm::vec3 worldPos = glm::vec3((*transform)[3]);
		worldAABB._min = worldPos - glm::vec3(0.1f);
		worldAABB._max = worldPos + glm::vec3(0.1f);
		// std::cerr << "Warning: updateWorldAABB not implemented for shape type: " << static_cast<int>(type) << std::endl;
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
#include <zg/Entity.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/physics/AABB.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
using namespace zg::components::entities;
using zg::physics::AABB;
BoxShapeData::BoxShapeData(glm::vec3 halfExtents) : halfExtents(halfExtents) {}
JPH::ShapeRefC BoxShapeData::createJoltShape() const {
	JPH::BoxShapeSettings settings(JPH::Vec3(halfExtents.x, halfExtents.y, halfExtents.z));
	// Optional: Add convex radius for better stability/performance trade-off
	// settings.mConvexRadius = 0.05f;
	auto result = settings.Create();
	if (result.HasError()) {
		std::cerr << "Jolt ERROR creating BoxShape: " << result.GetError().c_str() << std::endl;
		return nullptr;
	}
	return result.Get();
}
JPH::ShapeRefC SphereShapeData::createJoltShape() const {
	JPH::SphereShapeSettings settings(radius);
	auto result = settings.Create();
	if (result.HasError()) {
		std::cerr << "Jolt ERROR creating SphereShape: " << result.GetError().c_str() << std::endl;
		return nullptr;
	}
	return result.Get();
}
MeshShapeData::MeshShapeData(Entity& entity) : entity(entity) {}
JPH::ShapeRefC MeshShapeData::createJoltShape() const {
	std::cerr << "Jolt WARNING: MeshShape creation not implemented!" << std::endl;
	// Placeholder: Create a small box instead
	return BoxShapeData({0.1f, 0.1f, 0.1f}).createJoltShape();
	// --- Actual Implementation ---
	// 1. Get vertex and index data (e.g., from entity's MeshComponent)
	// JPH::VertexList vertices;
	// JPH::IndexedTriangleList triangles;
	// // ... populate vertices and triangles ...
	// JPH::MeshShapeSettings settings(vertices, triangles);
	// settings.SetEmbedded(); // Embed data in shape if desired
	// auto result = settings.Create();
	// if (result.HasError()) { /* handle error */ return nullptr; }
	// return result.Get();
}
Collider::Collider(const ColliderInfo& info) :
		IEntityComponent("Collider"), info(info),
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
	std::cout << "Collider attached." << std::endl;
}
void Collider::onUpdate()
{
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
	ownerRigidBody = 0;
	std::cout << "Collider detached." << std::endl;
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
zg::physics::AABB<3>& Collider::getWorldAABB()
{
	return worldAABB;
};
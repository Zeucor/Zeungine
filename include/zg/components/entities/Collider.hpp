#pragma once
#include <zg/glm.hpp>
#include <zg/interfaces/IEntityComponent.hpp>
#include <zg/physics/AABB.hpp>
namespace zg
{
	struct Entity;
}
namespace zg::components::entities
{
	struct RigidBody;
	enum class ShapeType
	{
		Box = 0,
		Sphere,
		Capsule,
		Mesh,
		ConvexHull,
		_Count // unused by shapes, used for getting the total count of ShapeTypes
	};
	struct ShapeData
	{
		virtual ~ShapeData() = default;
		virtual ShapeType getType() const = 0;
	};
	struct BoxShapeData : ShapeData
	{
		glm::vec3 halfExtents; // Half-width, half-height, half-depth
		ShapeType getType() const override { return ShapeType::Box; }
	};
	struct SphereShapeData : ShapeData
	{
		float radius;
		ShapeType getType() const override { return ShapeType::Sphere; }
	};
	struct CapsuleShapeData : ShapeData
	{
		float radius;
		float height; // Height of the cylindrical part
		ShapeType getType() const override { return ShapeType::Capsule; }
	};
	struct MeshShapeData : ShapeData
	{
		ShapeType getType() const override { return ShapeType::Mesh; }
	};
	struct ConvexHullShapeData : ShapeData
	{
		ShapeType getType() const override { return ShapeType::ConvexHull; }
	};
	struct PhysicsMaterial
	{
		float friction = 0.5f; // Coefficient of friction (0=slippery, 1+ = high friction)
		float restitution = 0.2f; // Bounciness (0=inelastic, 1=perfectly elastic)
	};
	struct ColliderInfo
	{
		Entity& entity;
		std::shared_ptr<ShapeData> shapeData;
		PhysicsMaterial material;
		glm::vec3 offset = {0, 0, 0}; // Local offset from the RigidBody's center/entity origin
		glm::quat rotationOffset = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Local rotation offset
		bool isSensor = false; // Trigger only (detects overlaps/collisions but no physical response)
	};
	struct Collider : interfaces::IEntityComponent
	{
	private:
		ColliderInfo info;
		RigidBody* ownerRigidBody = 0; // Pointer to the RB this collider is attached to
		glm::mat4* transform = 0; // Pointer to the entity's transform (for AABB updates)
        physics::AABB worldAABB;
		long double& deltaTime;

	public:
		Collider(const ColliderInfo& info);
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
		void updateWorldAABB();
		ShapeType getShapeType() const;
		PhysicsMaterial& getPhysicsMaterial();
		const PhysicsMaterial& getPhysicsMaterial() const;
		glm::vec3& getOffset();
		const glm::vec3& getOffset() const;
		glm::quat& getRotationOffset();
		const glm::quat& getRotationOffset() const;
		bool getIsSensor();
		ColliderInfo& getColliderInfo();
		RigidBody* getOwnerRigidBody();
		const glm::mat4* getTransform();
		physics::AABB& getWorldAABB();
		const physics::AABB& getWorldAABB() const;
	};
} // namespace zg::components::entities

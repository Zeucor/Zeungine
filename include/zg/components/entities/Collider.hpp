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
		virtual glm::vec3 getHalfExtents() const = 0;
		virtual glm::mat3 calculateInverseInertiaBody(float mass) = 0;
		physics::AABB getLocalAABB()
		{
			auto he = getHalfExtents();
			return {-he, he};
		}
	};
	struct BoxShapeData : ShapeData
	{
		glm::vec3 halfExtents; // Half-width, half-height, half-depth
		BoxShapeData(glm::vec3 halfExtents);
		ShapeType getType() const override { return ShapeType::Box; }
		glm::vec3 getHalfExtents() const { return halfExtents; }
		glm::mat3 calculateInverseInertiaBody(float mass) override;
	};
	struct SphereShapeData : ShapeData
	{
		float radius;
		ShapeType getType() const override { return ShapeType::Sphere; }
		glm::vec3 getHalfExtents() const { return glm::vec3(radius); }
		glm::mat3 calculateInverseInertiaBody(float mass) override;
	};
	struct CapsuleShapeData : ShapeData
	{
		float radius;
		float height; // Height of the cylindrical part
		ShapeType getType() const override { return ShapeType::Capsule; }
		glm::vec3 getHalfExtents() const { return glm::vec3(radius, (height / 2.f) + radius, radius); }
		glm::mat3 calculateInverseInertiaBody(float mass) override;
	};
	struct MeshShapeData : ShapeData
	{
		Entity& entity;
		MeshShapeData(Entity& entity);
		ShapeType getType() const override { return ShapeType::Mesh; }
		glm::vec3 getHalfExtents() const { /* TODO: Find half extents */ return glm::vec3(1); }
		glm::mat3 calculateInverseInertiaBody(float mass) override;
	};
	struct ConvexHullShapeData : ShapeData
	{
		ShapeType getType() const override { return ShapeType::ConvexHull; }
		glm::vec3 getHalfExtents() const { /* TODO: Find half extents */ return glm::vec3(1); }
		glm::mat3 calculateInverseInertiaBody(float mass) override;
	};
	struct PhysicsMaterial
	{
		float friction = 0.8f; // Coefficient of friction (0=slippery, 1+ = high friction)
		float restitution = 0.f; // Bounciness (0=inelastic, 1=perfectly elastic)
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
		physics::AABB getSweptWorldAABB(float ldt) const;
		physics::AABB calculateWorldAABB(const glm::vec3& center, const glm::quat& rotation) const;
		// getters
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
		const RigidBody* getOwnerRigidBody() const;
		const glm::mat4* getTransform();
		const physics::AABB& getWorldAABB() const;
		virtual glm::vec3 getCenterAtTime(float t) const;
	};
} // namespace zg::components::entities

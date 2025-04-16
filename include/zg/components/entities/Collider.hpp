#pragma once
#include <zg/glm.hpp>
#include <zg/interfaces/IEntityComponent.hpp>
#include <zg/physics/AABB.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
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
		virtual JPH::ShapeRefC createJoltShape() const = 0;
		physics::AABB<3> getLocalAABB()
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
		JPH::ShapeRefC createJoltShape() const override;
	};
	struct SphereShapeData : ShapeData
	{
		float radius;
		ShapeType getType() const override { return ShapeType::Sphere; }
		glm::vec3 getHalfExtents() const { return glm::vec3(radius); }
		JPH::ShapeRefC createJoltShape() const override;
	};
	struct CapsuleShapeData : ShapeData
	{
		float radius;
		float height; // Height of the cylindrical part
		ShapeType getType() const override { return ShapeType::Capsule; }
		glm::vec3 getHalfExtents() const { return glm::vec3(radius, (height / 2.f) + radius, radius); }
		JPH::ShapeRefC createJoltShape() const override;
	};
	struct MeshShapeData : ShapeData
	{
		Entity& entity;
		MeshShapeData(Entity& entity);
		ShapeType getType() const override { return ShapeType::Mesh; }
		glm::vec3 getHalfExtents() const { /* TODO: Find half extents */ return glm::vec3(1); }
		JPH::ShapeRefC createJoltShape() const override;
	};
	struct ConvexHullShapeData : ShapeData
	{
		ShapeType getType() const override { return ShapeType::ConvexHull; }
		glm::vec3 getHalfExtents() const { /* TODO: Find half extents */ return glm::vec3(1); }
		JPH::ShapeRefC createJoltShape() const override;
	};
	struct zgPhysicsMaterial
	{
		float friction = 0.8f; // Coefficient of friction (0=slippery, 1+ = high friction)
		float restitution = 0.f; // Bounciness (0=inelastic, 1=perfectly elastic)
	};
	struct ColliderInfo
	{
		Entity& entity;
		std::shared_ptr<ShapeData> shapeData;
		zgPhysicsMaterial material;
		glm::vec3 offset = {0, 0, 0};
		glm::quat rotationOffset = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		bool isSensor = false;
	};
	struct Collider : interfaces::IEntityComponent
	{
	private:
		ColliderInfo info;
		RigidBody* ownerRigidBody = 0;
		physics::AABB<3> worldAABB;
		long double& deltaTime;

	public:
		Collider(const ColliderInfo& info);
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
		// getters
		ShapeType getShapeType() const;
		zgPhysicsMaterial& getPhysicsMaterial();
		const zgPhysicsMaterial& getPhysicsMaterial() const;
		glm::vec3& getOffset();
		const glm::vec3& getOffset() const;
		glm::quat& getRotationOffset();
		const glm::quat& getRotationOffset() const;
		bool getIsSensor();
		ColliderInfo& getColliderInfo();
		RigidBody* getOwnerRigidBody();
		const RigidBody* getOwnerRigidBody() const;
		physics::AABB<3>& getWorldAABB();
	};
} // namespace zg::components::entities

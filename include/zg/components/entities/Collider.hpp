#pragma once
#include <zg/glm.hpp>
#include "EntityComponent.hpp"
#include <zg/physics/AABB.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
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
		virtual JPH::ShapeRefC createJoltShape(Entity& entity) const = 0;
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
		JPH::ShapeRefC createJoltShape(Entity& entity) const override;
	};
	struct SphereShapeData : ShapeData
	{
		float radius;
		ShapeType getType() const override { return ShapeType::Sphere; }
		glm::vec3 getHalfExtents() const { return glm::vec3(radius); }
		JPH::ShapeRefC createJoltShape(Entity& entity) const override;
	};
	struct CapsuleShapeData : ShapeData
	{
		float radius;
		float height; // Height of the cylindrical part
		ShapeType getType() const override { return ShapeType::Capsule; }
		glm::vec3 getHalfExtents() const { return glm::vec3(radius, (height / 2.f) + radius, radius); }
		JPH::ShapeRefC createJoltShape(Entity& entity) const override;
	};
	struct MeshShapeData : ShapeData
	{
		MeshShapeData() = default;
		ShapeType getType() const override { return ShapeType::Mesh; }
		glm::vec3 getHalfExtents() const { /* TODO: Find half extents */ return glm::vec3(1); }
		JPH::ShapeRefC createJoltShape(Entity& entity) const override;
	};
	struct ConvexHullShapeData : ShapeData
	{
		ConvexHullShapeData() = default;
		ShapeType getType() const override { return ShapeType::ConvexHull; }
		glm::vec3 getHalfExtents() const { /* TODO: Find half extents */ return glm::vec3(1); }
		JPH::ShapeRefC createJoltShape(Entity& entity) const override;
	};
	struct PhysicsMaterial
	{
		float friction = 0.8f; // Coefficient of friction (0=slippery, 1+ = high friction)
		float restitution = 0.f; // Bounciness (0=inelastic, 1=perfectly elastic)
	};
	struct ColliderInfo
	{
		std::shared_ptr<ShapeData> shapeData;
		PhysicsMaterial material;
		bool isSensor = false;
		ColliderInfo(const std::shared_ptr<ShapeData>& shapeData, const PhysicsMaterial& material, bool isSensor);
		ColliderInfo(const ColliderInfo& other);
		ColliderInfo& operator=(const ColliderInfo& other);
	};
	// struct Collider : components::entities::EntityComponent
	// {
	// private:
	// 	ColliderInfo info;
	// 	RigidBody* ownerRigidBody = 0;
	// 	physics::AABB<3> worldAABB;
	// 	long double& deltaTime;

	// public:
	// 	Collider(const ColliderInfo& info);
	// 	void onAttached() override;
	// 	void onUpdate() override;
	// 	void onDetached() override;
	// 	// getters
	// 	ShapeType getShapeType() const;
	// 	PhysicsMaterial& getPhysicsMaterial();
	// 	const PhysicsMaterial& getPhysicsMaterial() const;
	// 	glm::vec3& getOffset();
	// 	const glm::vec3& getOffset() const;
	// 	glm::quat& getRotationOffset();
	// 	const glm::quat& getRotationOffset() const;
	// 	bool getIsSensor();
	// 	ColliderInfo& getColliderInfo();
	// 	RigidBody* getOwnerRigidBody();
	// 	const RigidBody* getOwnerRigidBody() const;
	// 	physics::AABB<3>& getWorldAABB();
	// };
	components::entities::EntityComponentCreateInfo ColliderFactory(const ColliderInfo& colliderInfo);
} // namespace zg::components::entities

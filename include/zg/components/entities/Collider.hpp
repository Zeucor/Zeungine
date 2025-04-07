#pragma once
#include <zg/interfaces/IEntityComponent.hpp>
#include <zg/glm.hpp>
namespace zg
{
	struct Entity;
}
namespace zg::components::entities
{
    struct RigidBody;
	enum class ShapeType
	{
		Box,
		Sphere,
		Capsule,
		Mesh,
		ConvexHull,
		_Count
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
        float friction = 0.5f;    // Coefficient of friction (0=slippery, 1+ = high friction)
        float restitution = 0.2f; // Bounciness (0=inelastic, 1=perfectly elastic)
    };
    struct ColliderInfo
    {
		Entity& entity;
        std::shared_ptr<ShapeData> shapeData;
        PhysicsMaterial material;
        glm::vec3 offset = {0,0,0}; // Local offset from the RigidBody's center/entity origin
        glm::quat rotationOffset = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Local rotation offset
        bool isSensor = false; // Trigger only (detects overlaps/collisions but no physical response)
    };
	struct Collider : interfaces::IEntityComponent
	{
        ColliderInfo info;
        RigidBody* ownerRigidBody = 0; // Pointer to the RB this collider is attached to
        const glm::mat4* transform = 0; // Pointer to the entity's transform (for AABB updates)
		Collider(const ColliderInfo& info);
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
	};
} // namespace zg::components::entities

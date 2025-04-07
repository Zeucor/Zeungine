#pragma once
#include <zg/glm.hpp>
#include <zg/interfaces/IEntityComponent.hpp>
#include <zg/glm.hpp>
namespace zg
{
	struct Entity;
}
namespace zg::components::scenes
{
	struct PhysicsScene;
}
namespace zg::components::entities
{
	enum class BodyType
	{
		Static, // Infinite mass, doesn't move, unaffected by forces (e.g., ground)
		Kinematic, // Infinite mass, moved via code (setPosition/setRotation), not forces (e.g., moving platforms)
		Dynamic // Finite mass, moved by forces and collisions
	};
	struct RigidBodyInfo
	{
		Entity& entity;
		BodyType bodyType;
		float mass = 1.0f;
		float linearDamping = 0.1f;
		float angularDamping = 0.05f;
		bool useGravity = true;
		bool isKinematicInitially = false;
        glm::vec<3, bool> freezeRotationAxes = {false, false, false};
        glm::vec<3, bool> freezeVelocityAxes = {false, false, false};
	};
	struct Collider;
	struct RigidBody : interfaces::IEntityComponent
	{
		RigidBodyInfo info;
		glm::mat4* transform = 0;
		scenes::PhysicsScene* physicsScene = 0;
        std::vector<Collider*> colliders;
        glm::vec3 linearVelocity = {0.0f, 0.0f, 0.0f};
        glm::vec3 angularVelocity = {0.0f, 0.0f, 0.0f};
        glm::vec3 forceAccumulator = {0.0f, 0.0f, 0.0f};
        glm::vec3 torqueAccumulator = {0.0f, 0.0f, 0.0f};
        float inverseMass = 0.0f;
		RigidBody(const RigidBodyInfo& info);
        void addCollider(Collider* collider);
        void removeCollider(Collider* collider);
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
        void applyForce(glm::vec3 force, glm::vec3 worldPoint); // Apply force at world point
        void applyForceToCenter(glm::vec3 force); // Apply force at center of mass
        void applyTorque(glm::vec3 torque); // Apply rotational force
        void clearForces(); // Clears force and torque accumulators (called by Physics system each step)
        bool isStatic() const;
        bool isKinematic() const;
        bool isDynamic() const;
	};
} // namespace zg::components::entities

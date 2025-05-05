#pragma once
#include <zg/glm.hpp>
#include "EntityComponent.hpp"
#include <zg/physics/CollisionManifold.hpp>
#include <zg/conversions/ToJolt.hpp>
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
		BodyType bodyType = BodyType::Dynamic;
		float mass = 1.0f;
		float linearDamping = 0.8f;
		float angularDamping = 0.8f;
		bool useGravity = true;
		bool isKinematicInitially = false;
		glm::vec<3, bool> freezeRotationAxes = {false, false, false};
		glm::vec<3, bool> freezeVelocityAxes = {false, false, false};
		size_t collisionMask = 1;
		RigidBodyInfo() = default;
		RigidBodyInfo(
			BodyType bodyType,
			float mass = 1.0f,
			float linearDamping = 0.8f,
			float angularDamping = 0.8f,
			bool useGravity = true,
			bool isKinematicInitially = false,
			glm::vec<3, bool> freezeRotationAxes = {false, false, false},
			glm::vec<3, bool> freezeVelocityAxes = {false, false, false},
			size_t collisionMask = 1);
		RigidBodyInfo(const RigidBodyInfo& other);
		RigidBodyInfo& operator=(const RigidBodyInfo& other);
	};
	components::entities::EntityComponentCreateInfo RigidBodyFactory(const RigidBodyInfo& info);
} // namespace zg::components::entities

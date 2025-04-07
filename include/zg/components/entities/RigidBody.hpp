#pragma once
#include <zg/glm.hpp>
#include <zg/interfaces/IEntityComponent.hpp>
#include <zg/glm.hpp>
namespace zg
{
	struct Entity;
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
        glm::vec<3, bool> freezeTorqueAxes = {false, false, false};
        glm::vec<3, bool> freezeAccellAxes = {false, false, false};
	};
	struct RigidBody : interfaces::IEntityComponent
	{
		RigidBodyInfo info;
		const glm::mat4* transform = 0;
		RigidBody(const RigidBodyInfo& info);
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
	};
} // namespace zg::components::entities

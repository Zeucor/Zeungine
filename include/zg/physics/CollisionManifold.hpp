#pragma once
#include <vector>
#include <zg/glm.hpp>
#include <zg/components/entities/EntityComponent.hpp>
namespace zg::components::entities
{
	struct RigidBody;
} // namespace zg::components::entities

namespace zg::physics
{
	// Structure to hold information about a collision contact
	// Simplified version without multiple contact points for now
	struct CollisionManifold
	{
		components::entities::EntityComponent* ecA = nullptr;
		components::entities::EntityComponent* ecB = nullptr;
		glm::vec3 normal = glm::vec3(0.0f); // From B's perspective (points from B to A, i.e., push A along normal)
		float penetrationDepth = 0.0f; // Minimum penetration depth along the normal
		bool colliding = false;
		std::vector<glm::vec3> worldContactPointsOnA;
		std::vector<glm::vec3> worldContactPointsOnB;
		std::vector<glm::vec3> relativeContactPointsOnA;
		std::vector<glm::vec3> relativeContactPointsOnB;
		CollisionManifold() = default;
		CollisionManifold(components::entities::EntityComponent* a, components::entities::EntityComponent* b) : ecA(a), ecB(b) {}
	};
} // namespace zg::physics

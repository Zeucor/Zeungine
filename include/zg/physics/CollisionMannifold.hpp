#pragma once
#include <vector>
#include <zg/glm.hpp>

namespace zg::components::entities
{
	struct Collider;
} // namespace zg::components::entities

namespace zg::physics
{
	// Structure to hold information about a collision contact
	// Simplified version without multiple contact points for now
	struct CollisionMannifold
	{
		components::entities::Collider* colliderA = nullptr;
		components::entities::Collider* colliderB = nullptr;
		glm::vec3 normal = glm::vec3(0.0f); // From B's perspective (points from B to A, i.e., push A along normal)
		float penetrationDepth = 0.0f; // Minimum penetration depth along the normal
		bool colliding = false;
		std::vector<glm::vec3> contactPoints;
		size_t contactCount = 0;
		CollisionMannifold() = default;
		CollisionMannifold(components::entities::Collider* a, components::entities::Collider* b) : colliderA(a), colliderB(b) {}
	};
} // namespace zg::physics

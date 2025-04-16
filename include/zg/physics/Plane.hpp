#pragma once
#include <zg/glm.hpp>
namespace zg::physics
{
	struct Plane
	{
		glm::vec3 normal;
		float distance; // Distance from origin along the normal

		Plane(const glm::vec3& n, float d) : normal(glm::normalize(n)), distance(d) {}
		Plane(const glm::vec3& n, const glm::vec3& pointOnPlane) : normal(glm::normalize(n))
		{
			distance = glm::dot(normal, pointOnPlane);
		}

		float signedDistance(const glm::vec3& point) const { return glm::dot(normal, point) - distance; }

		float distanceToPoint(const glm::vec3& point) const
		{
			// Positive distance means point is on the side the normal points to
			return glm::dot(point, normal) - distance;
		}
	};
} // namespace zg::physics

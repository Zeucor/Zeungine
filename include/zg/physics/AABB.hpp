
#pragma once
#include <algorithm> // For std::_min/_max used by glm::min/_max
#include <limits> // For infinity
#include <zg/glm.hpp>

namespace zg::physics
{
	// Axis-Aligned Bounding Box structure
	struct AABB
	{
		glm::vec3 _min = glm::vec3(std::numeric_limits<float>::infinity());
		glm::vec3 _max = glm::vec3(-std::numeric_limits<float>::infinity());

		AABB() = default;

		AABB(glm::vec3 _minPoint, glm::vec3 _maxPoint) : _min(_minPoint), _max(_maxPoint) {}

		// Check for overlap with another AABB
		bool overlaps(const AABB& other) const
		{
			return (_max.x >= other._min.x && _min.x <= other._max.x && _max.y >= other._min.y && _min.y <= other._max.y &&
							_max.z >= other._min.z && _min.z <= other._max.z);
		}

		// Expand the AABB to include a point
		void encompass(glm::vec3 point)
		{
			_min = (glm::min)(_min, point);
			_max = (glm::max)(_max, point);
		}

		// Reset bounds to infinite values
		void reset()
		{
			_min = glm::vec3(std::numeric_limits<float>::infinity());
			_max = glm::vec3(-std::numeric_limits<float>::infinity());
		}
	};
} // namespace zg::physics

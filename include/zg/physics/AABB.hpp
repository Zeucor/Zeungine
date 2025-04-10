
#pragma once
#include <algorithm> // For std::_min/_max used by glm::min/_max
#include <limits> // For infinity
#include <zg/glm.hpp>

namespace zg::physics
{
	// Axis-Aligned Bounding Box structure
	struct AABB
	{
		enum class Overlaps
		{
			None = 0,
			X = 1,
			Y = 2,
			Z = 4
		};

		glm::vec3 _min = glm::vec3(std::numeric_limits<float>::infinity());
		glm::vec3 _max = glm::vec3(-std::numeric_limits<float>::infinity());

		AABB() = default;

		AABB(glm::vec3 _minPoint, glm::vec3 _maxPoint) : _min(_minPoint), _max(_maxPoint) {}

		// Check for overlap with another AABB
		Overlaps overlaps(const AABB& other) const
		{
			float minX1 = (std::min)(_min.x, _max.x);
			float maxX1 = (std::max)(_min.x, _max.x);
			float minY1 = (std::min)(_min.y, _max.y);
			float maxY1 = (std::max)(_min.y, _max.y);
			float minZ1 = (std::min)(_min.z, _max.z);
			float maxZ1 = (std::max)(_min.z, _max.z);
		
			float minX2 = (std::min)(other._min.x, other._max.x);
			float maxX2 = (std::max)(other._min.x, other._max.x);
			float minY2 = (std::min)(other._min.y, other._max.y);
			float maxY2 = (std::max)(other._min.y, other._max.y);
			float minZ2 = (std::min)(other._min.z, other._max.z);
			float maxZ2 = (std::max)(other._min.z, other._max.z);
		
			uint32_t overlaps = (uint32_t)Overlaps::None;

			if (minX2 >= minX1 && maxX1 >= maxX2)
			{
				overlaps |= (uint32_t)Overlaps::X;
			}
			if (minY2 >= minY1 && maxY1 >= minY2)
			{
				overlaps |= (uint32_t)Overlaps::Y;
			}
			if (minZ2 >= minZ1 && maxZ1 >= maxZ2)
			{
				overlaps |= (uint32_t)Overlaps::Z;
			}

			// if (maxX1 >= minX2 && minX1 <= maxX2)
			// {
			// 	overlaps |= (uint32_t)Overlaps::X;
			// }
			// if (maxY1 >= minY2 && minY1 <= maxY2)
			// {
			// 	overlaps |= (uint32_t)Overlaps::Y;
			// }
			// if (maxZ1 >= minZ2 && minZ1 <= maxZ2)
			// {
			// 	overlaps |= (uint32_t)Overlaps::Z;
			// }

			return (Overlaps)overlaps;
		}

		static bool overlapsAll( Overlaps overlaps)
		{
			return (uint32_t)overlaps == 7;
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

		static AABB merge(const AABB &a, const AABB &b)
		{
			AABB result;
			result._min = (glm::min)(a._min, b._min);
			result._max = (glm::max)(a._max, b._max);
			return result;
		}
	};
} // namespace zg::physics

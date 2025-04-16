
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
			Left = 1,
			Bottom = 2,
			Front = 4,
			Right = 8,
			Top = 16,
			Back = 32
		};

		glm::vec3 _min = glm::vec3(std::numeric_limits<float>::infinity());
		glm::vec3 _max = glm::vec3(-std::numeric_limits<float>::infinity());

		AABB() = default;

		AABB(glm::vec3 _minPoint, glm::vec3 _maxPoint) : _min(_minPoint), _max(_maxPoint) {}

		// Check for overlap with another AABB
		Overlaps overlaps(const AABB& other) const
		{
			uint32_t currentOverlaps = (uint32_t)Overlaps::None;
			bool overlapX = (_min.x <= other._max.x && _max.x >= other._min.x);
			bool overlapY = (_min.y <= other._max.y && _max.y >= other._min.y);
			bool overlapZ = (_min.z <= other._max.z && _max.z >= other._min.z);
			if (!(overlapX && overlapY && overlapZ))
			{
				return (Overlaps)currentOverlaps;
			}
			glm::vec3 thisCenter = _min + (_max - _min) * 0.5f;
			glm::vec3 otherCenter = other._min + (other._max - other._min) * 0.5f;
			glm::vec3 centerDiff = otherCenter - thisCenter;
			float penX = ((_max.x - _min.x) + (other._max.x - other._min.x)) * 0.5f - std::abs(centerDiff.x);
			float penY = ((_max.y - _min.y) + (other._max.y - other._min.y)) * 0.5f - std::abs(centerDiff.y);
			float penZ = ((_max.z - _min.z) + (other._max.z - other._min.z)) * 0.5f - std::abs(centerDiff.z);
			float minPen = (std::numeric_limits<float>::max)();
			if (penX > 0 && penX < minPen)
			{
				minPen = penX;
				if (centerDiff.x > 0)
				{
					currentOverlaps = (uint32_t)Overlaps::Right;
				}
				else
				{
					currentOverlaps = (uint32_t)Overlaps::Left;
				}
			}
			if (penY > 0 && penY < minPen)
			{
				minPen = penY;
				if (centerDiff.y > 0)
				{
					currentOverlaps = (uint32_t)Overlaps::Top;
				}
				else
				{
					currentOverlaps = (uint32_t)Overlaps::Bottom;
				}
			}
			if (penZ > 0 && penZ < minPen)
			{
				if (centerDiff.z > 0)
				{
					currentOverlaps = (uint32_t)Overlaps::Back;
				}
				else
				{
					currentOverlaps = (uint32_t)Overlaps::Front;
				}
			}
			return (Overlaps)currentOverlaps;
		}
		Overlaps overlaps(const AABB& other, glm::vec3& normal, float& penetrationDepth, float& minDistance, int& axis) const
		{
			uint32_t currentOverlaps = (uint32_t)Overlaps::None;
			float distX = (std::max)(0.0f, (std::max)(_min.x - other._max.x, other._min.x - _max.x));
			float distY = (std::max)(0.0f, (std::max)(_min.y - other._max.y, other._min.y - _max.y));
			float distZ = (std::max)(0.0f, (std::max)(_min.z - other._max.z, other._min.z - _max.z));
			minDistance = std::sqrt(distX * distX + distY * distY + distZ * distZ);
			bool overlapX = (_min.x <= other._max.x && _max.x >= other._min.x);
			bool overlapY = (_min.y <= other._max.y && _max.y >= other._min.y);
			bool overlapZ = (_min.z <= other._max.z && _max.z >= other._min.z);
			glm::vec3 thisCenter = _min + (_max - _min) * 0.5f;
			glm::vec3 otherCenter = other._min + (other._max - other._min) * 0.5f;
			glm::vec3 centerDiff = otherCenter - thisCenter;
			float penX = ((_max.x - _min.x) + (other._max.x - other._min.x)) * 0.5f - std::abs(centerDiff.x);
			float penY = ((_max.y - _min.y) + (other._max.y - other._min.y)) * 0.5f - std::abs(centerDiff.y);
			float penZ = ((_max.z - _min.z) + (other._max.z - other._min.z)) * 0.5f - std::abs(centerDiff.z);
			float minPen = (std::numeric_limits<float>::max)();
			axis = -1;
			if (penX < minPen)
			{
				minPen = penX;
				axis = 0;
			}
			if (penY < minPen)
			{
				minPen = penY;
				axis = 1;
			}
			if (penZ < minPen)
			{
				minPen = penZ;
				axis = 2;
			}
			if (axis == -1)
			{
				return (Overlaps)currentOverlaps;
			}
			penetrationDepth = minPen;
			if (axis == 0)
			{
				if (centerDiff.x > 0)
				{
					if (minPen >= 0)
						currentOverlaps = (uint32_t)Overlaps::Right;
					normal = {1.f, 0.f, 0.f};
					float contactX = (this->_max.x + other._min.x) * 0.5f;
					float minY = (std::max)(this->_min.y, other._min.y);
					float maxY = (std::min)(this->_max.y, other._max.y);
					float minZ = (std::max)(this->_min.z, other._min.z);
					float maxZ = (std::min)(this->_max.z, other._max.z);
				}
				else
				{
					if (minPen >= 0)
						currentOverlaps = (uint32_t)Overlaps::Left;
					normal = {-1.f, 0.f, 0.f};
					float contactX = (this->_min.x + other._max.x) * 0.5f;
					float minY = (std::max)(this->_min.y, other._min.y);
					float maxY = (std::min)(this->_max.y, other._max.y);
					float minZ = (std::max)(this->_min.z, other._min.z);
					float maxZ = (std::min)(this->_max.z, other._max.z);
				}
			}
			else if (axis == 1)
			{
				if (centerDiff.y > 0)
				{
					if (minPen >= 0)
						currentOverlaps = (uint32_t)Overlaps::Top;
					normal = {0.f, 1.f, 0.f};
					float contactY = (this->_max.y + other._min.y) * 0.5f;
					float minX = (std::max)(this->_min.x, other._min.x);
					float maxX = (std::min)(this->_max.x, other._max.x);
					float minZ = (std::max)(this->_min.z, other._min.z);
					float maxZ = (std::min)(this->_max.z, other._max.z);
				}
				else
				{
					if (minPen >= 0)
						currentOverlaps = (uint32_t)Overlaps::Bottom;
					normal = {0.f, -1.f, 0.f};
					float contactY = (this->_min.y + other._max.y) * 0.5f;
					float minX = (std::max)(this->_min.x, other._min.x);
					float maxX = (std::min)(this->_max.x, other._max.x);
					float minZ = (std::max)(this->_min.z, other._min.z);
					float maxZ = (std::min)(this->_max.z, other._max.z);
				}
			}
			else
			{
				if (centerDiff.z > 0)
				{
					if (minPen >= 0)
						currentOverlaps = (uint32_t)Overlaps::Back;
					normal = {0.f, 0.f, 1.f};
					float contactZ = (this->_max.z + other._min.z) * 0.5f;
					float minX = (std::max)(this->_min.x, other._min.x);
					float maxX = (std::min)(this->_max.x, other._max.x);
					float minY = (std::max)(this->_min.y, other._min.y);
					float maxY = (std::min)(this->_max.y, other._max.y);
				}
				else
				{
					if (minPen >= 0)
						currentOverlaps = (uint32_t)Overlaps::Front;
					normal = {0.f, 0.f, -1.f};
					float contactZ = (this->_min.z + other._max.z) * 0.5f;
					float minX = (std::max)(this->_min.x, other._min.x);
					float maxX = (std::min)(this->_max.x, other._max.x);
					float minY = (std::max)(this->_min.y, other._min.y);
					float maxY = (std::min)(this->_max.y, other._max.y);
				}
			}
			return (Overlaps)currentOverlaps;
		}
		Overlaps overlaps(const AABB& other, glm::vec3& normal, float& penetrationDepth, float& minDistance,
											std::vector<glm::vec3>& contactPoints) const
		{
			uint32_t currentOverlaps = (uint32_t)Overlaps::None;
			float distX = (std::max)(0.0f, (std::max)(_min.x - other._max.x, other._min.x - _max.x));
			float distY = (std::max)(0.0f, (std::max)(_min.y - other._max.y, other._min.y - _max.y));
			float distZ = (std::max)(0.0f, (std::max)(_min.z - other._max.z, other._min.z - _max.z));
			minDistance = std::sqrt(distX * distX + distY * distY + distZ * distZ);
			normal = {0.f, 0.f, 0.f};
			penetrationDepth = 0.f;
			contactPoints.clear();
			bool overlapX = (_min.x <= other._max.x && _max.x >= other._min.x);
			bool overlapY = (_min.y <= other._max.y && _max.y >= other._min.y);
			bool overlapZ = (_min.z <= other._max.z && _max.z >= other._min.z);
			if (!(overlapX && overlapY && overlapZ))
			{
				return (Overlaps)currentOverlaps;
			}
			glm::vec3 thisCenter = _min + (_max - _min) * 0.5f;
			glm::vec3 otherCenter = other._min + (other._max - other._min) * 0.5f;
			glm::vec3 centerDiff = otherCenter - thisCenter;
			float penX = ((_max.x - _min.x) + (other._max.x - other._min.x)) * 0.5f - std::abs(centerDiff.x);
			float penY = ((_max.y - _min.y) + (other._max.y - other._min.y)) * 0.5f - std::abs(centerDiff.y);
			float penZ = ((_max.z - _min.z) + (other._max.z - other._min.z)) * 0.5f - std::abs(centerDiff.z);
			float minPen = (std::numeric_limits<float>::max)();
			int axis = -1;
			if (penX >= 0 && penX <= minPen)
			{
				minPen = penX;
				axis = 0;
			}
			if (penY >= 0 && penY <= minPen)
			{
				minPen = penY;
				axis = 1;
			}
			if (penZ >= 0 && penZ <= minPen)
			{
				minPen = penZ;
				axis = 2;
			}
			if (axis == -1)
			{
				return (Overlaps)currentOverlaps;
			}
			penetrationDepth = minPen;
			if (axis == 0)
			{
				if (centerDiff.x >= 0)
				{
					currentOverlaps = (uint32_t)Overlaps::Right;
					normal = {1.f, 0.f, 0.f};
					float contactX = (this->_max.x + other._min.x + minPen) * 0.5f;
					float minY = (std::max)(this->_min.y, other._min.y);
					float maxY = (std::min)(this->_max.y, other._max.y);
					float minZ = (std::max)(this->_min.z, other._min.z);
					float maxZ = (std::min)(this->_max.z, other._max.z);
					contactPoints.emplace_back(contactX, minY, minZ);
					contactPoints.emplace_back(contactX, maxY, minZ);
					contactPoints.emplace_back(contactX, minY, maxZ);
					contactPoints.emplace_back(contactX, maxY, maxZ);
				}
				else
				{
					currentOverlaps = (uint32_t)Overlaps::Left;
					normal = {-1.f, 0.f, 0.f};
					float contactX = (this->_min.x + other._max.x + minPen) * 0.5f;
					float minY = (std::max)(this->_min.y, other._min.y);
					float maxY = (std::min)(this->_max.y, other._max.y);
					float minZ = (std::max)(this->_min.z, other._min.z);
					float maxZ = (std::min)(this->_max.z, other._max.z);
					contactPoints.emplace_back(contactX, minY, minZ);
					contactPoints.emplace_back(contactX, maxY, minZ);
					contactPoints.emplace_back(contactX, minY, maxZ);
					contactPoints.emplace_back(contactX, maxY, maxZ);
				}
			}
			else if (axis == 1)
			{
				if (centerDiff.y >= 0)
				{
					currentOverlaps = (uint32_t)Overlaps::Top;
					normal = {0.f, 1.f, 0.f};
					float contactY = (this->_max.y + other._min.y + minPen) * 0.5f;
					float minX = (std::max)(this->_min.x, other._min.x);
					float maxX = (std::min)(this->_max.x, other._max.x);
					float minZ = (std::max)(this->_min.z, other._min.z);
					float maxZ = (std::min)(this->_max.z, other._max.z);
					contactPoints.emplace_back(minX, contactY, minZ);
					contactPoints.emplace_back(maxX, contactY, minZ);
					contactPoints.emplace_back(minX, contactY, maxZ);
					contactPoints.emplace_back(maxX, contactY, maxZ);
				}
				else
				{
					currentOverlaps = (uint32_t)Overlaps::Bottom;
					normal = {0.f, -1.f, 0.f};
					float contactY = (this->_min.y + minPen + other._max.y) * 0.5f;
					float minX = (std::max)(this->_min.x, other._min.x);
					float maxX = (std::min)(this->_max.x, other._max.x);
					float minZ = (std::max)(this->_min.z, other._min.z);
					float maxZ = (std::min)(this->_max.z, other._max.z);
					contactPoints.emplace_back(minX, contactY, minZ);
					contactPoints.emplace_back(maxX, contactY, minZ);
					contactPoints.emplace_back(minX, contactY, maxZ);
					contactPoints.emplace_back(maxX, contactY, maxZ);
				}
			}
			else
			{
				if (centerDiff.z >= 0)
				{
					currentOverlaps = (uint32_t)Overlaps::Back;
					normal = {0.f, 0.f, 1.f};
					float contactZ = (this->_max.z + other._min.z) * 0.5f;
					float minX = (std::max)(this->_min.x, other._min.x);
					float maxX = (std::min)(this->_max.x, other._max.x);
					float minY = (std::max)(this->_min.y, other._min.y);
					float maxY = (std::min)(this->_max.y, other._max.y);
					contactPoints.emplace_back(minX, minY, contactZ);
					contactPoints.emplace_back(maxX, minY, contactZ);
					contactPoints.emplace_back(minX, maxY, contactZ);
					contactPoints.emplace_back(maxX, maxY, contactZ);
				}
				else
				{
					currentOverlaps = (uint32_t)Overlaps::Front;
					normal = {0.f, 0.f, -1.f};
					float contactZ = (this->_min.z + other._max.z) * 0.5f;
					float minX = (std::max)(this->_min.x, other._min.x);
					float maxX = (std::min)(this->_max.x, other._max.x);
					float minY = (std::max)(this->_min.y, other._min.y);
					float maxY = (std::min)(this->_max.y, other._max.y);
					contactPoints.emplace_back(minX, minY, contactZ);
					contactPoints.emplace_back(maxX, minY, contactZ);
					contactPoints.emplace_back(minX, maxY, contactZ);
					contactPoints.emplace_back(maxX, maxY, contactZ);
				}
			}
			return (Overlaps)currentOverlaps;
		}

		static bool overlapsAll(Overlaps overlaps) { return (uint32_t)overlaps == 7; }

		static bool overlapsN(Overlaps overlaps, size_t N)
		{
			auto i = (uint32_t)overlaps;
			auto c = 0;
			for (auto n = 1; n <= 32; n *= 2)
			{
				if (i & n)
					c++;
			}
			return c >= N;
		}

		static bool overlaps3Axis(Overlaps overlaps)
		{
			uint32_t i = (uint32_t)overlaps;
			return (((i & (uint32_t)Overlaps::Left) || (i & (uint32_t)Overlaps::Right)) &&
							((i & (uint32_t)Overlaps::Bottom) || (i & (uint32_t)Overlaps::Top)) &&
							((i & (uint32_t)Overlaps::Front) || (i & (uint32_t)Overlaps::Back)));
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

		static AABB merge(const AABB& a, const AABB& b)
		{
			AABB result;
			result._min = (glm::min)(a._min, b._min);
			result._max = (glm::max)(a._max, b._max);
			return result;
		}

		glm::vec3 getCenter() const { return (_min + _max) * 0.5f; }

		glm::vec3 getHalfExtents() const { return (_max - _min) * 0.5f; }
	};
} // namespace zg::physics

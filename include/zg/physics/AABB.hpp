
#pragma once
#include <algorithm> // For std::_min/_max used by glm::min/_max
#include <limits> // For infinity
#include <zg/glm.hpp>

namespace zg::physics
{
	// Axis-Aligned Bounding Box structure
	template <size_t N = 3>
	struct AABB
	{
		enum class Overlaps
		{
			None = 0,
			Right = 1,
			Left = 2,
			Top = 4,
			Bottom = 8,
			Back = 16,
			Front = 32,
			T1 = 64,
			T2 = 128,
			_Count = 128
		};

		glm::vec<N, float> _min = glm::vec<N, float>(std::numeric_limits<float>::infinity());
		glm::vec<N, float> _max = glm::vec<N, float>(-std::numeric_limits<float>::infinity());

		AABB() = default;

		AABB(glm::vec<N, float> _minPoint, glm::vec<N, float> _maxPoint) : _min(_minPoint), _max(_maxPoint) {}

		// Check for overlap with another AABB
		Overlaps overlaps(const AABB& other) const
		{
			uint32_t currentOverlaps = (uint32_t)Overlaps::None;
			bool overlapX = false, overlapY = false, overlapZ = false;
			bool overlap[N] = {false};
			bool allTrue = true;
			for (auto axis = 0; axis < N; ++axis)
			{
				if (!(overlap[axis] = (_min[axis] <= other._max[axis] && _max[axis] >= other._min[axis])) && allTrue)
				{
					allTrue = false;
				}
			}
			if (!allTrue)
			{
				return (Overlaps)currentOverlaps;
			}
			glm::vec<N, float> thisCenter = _min + (_max - _min) * 0.5f;
			glm::vec<N, float> otherCenter = other._min + (other._max - other._min) * 0.5f;
			glm::vec<N, float> centerDiff = otherCenter - thisCenter;
			float pen[N] = {0.f};
			float minPen = (std::numeric_limits<float>::max)();
			for (auto axis = 0; axis < N; ++axis)
			{
				pen[axis] =
					((_max[axis] - _min[axis]) + (other._max[axis] - other._min[axis])) * 0.5f - std::abs(centerDiff[axis]);
				if (pen[axis] > 0 && pen[axis] < minPen)
				{
					minPen = pen[axis];
					if (centerDiff[axis] > 0)
					{
						currentOverlaps = (uint32_t)(std::pow(2, (axis * 2) + 1));
					}
					else
					{
						currentOverlaps = (uint32_t)(std::pow(2, (axis * 2) + 2));
					}
				}
			}
			return (Overlaps)currentOverlaps;
		}
		Overlaps overlaps(const AABB& other, glm::vec<N, float>& normal, float& penetrationDepth, float& minDistance,
											int& axis) const
		{
			uint32_t currentOverlaps = (uint32_t)Overlaps::None;
			minDistance = 0.f;
			for (auto axs = 0; axs < N; ++axs)
			{
				auto distAxis = (std::max)(0.f, (std::max)(_min[axs] - other._min[axs], other._min[axs] - _max[axs]));
				minDistance += distAxis * distAxis;
			}
			minDistance = std::sqrt(minDistance);
			bool overlapX = false, overlapY = false, overlapZ = false;
			bool overlap[N] = {false};
			bool allTrue = true;
			for (auto axs = 0; axs < N; ++axs)
			{
				if (!(overlap[axs] = (_min[axs] <= other._max[axs] && _max[axs] >= other._min[axs])) && allTrue)
				{
					allTrue = false;
				}
			}
			if (!allTrue)
			{
				return (Overlaps)currentOverlaps;
			}
			glm::vec<N, float> thisCenter = _min + (_max - _min) * 0.5f;
			glm::vec<N, float> otherCenter = other._min + (other._max - other._min) * 0.5f;
			glm::vec<N, float> centerDiff = otherCenter - thisCenter;
			float pen[N] = {0.f};
			float minPen = (std::numeric_limits<float>::max)();
			for (auto axs = 0; axs < N; ++axs)
			{
				pen[axs] = ((_max[axs] - _min[axs]) + (other._max[axs] - other._min[axs])) * 0.5f - std::abs(centerDiff[axs]);
				if (pen[axs] > 0 && pen[axs] < minPen)
				{
					minPen = pen[axs];
					axis = axs;
					if (centerDiff[axs] > 0)
					{
						currentOverlaps = (uint32_t)(std::pow(2, (axs * 2) + 1));
					}
					else
					{
						currentOverlaps = (uint32_t)(std::pow(2, (axs * 2) + 2));
					}
				}
			}
			penetrationDepth = minPen;
			normal = glm::vec<N, float>(0);
			normal[axis] = (centerDiff[axis] > 0) ? 1.f : -1.f;
			return (Overlaps)currentOverlaps;
		}

		static bool overlapsNSides(Overlaps overlaps, size_t NSides)
		{
			auto i = (uint32_t)overlaps;
			auto c = 0;
			for (auto n = 1; n <= (uint32_t)Overlaps::_Count; n *= 2)
			{
				if (i & n)
					c++;
			}
			return c >= NSides;
		}

		static bool overlapsNAxis(Overlaps overlaps, size_t NAxis)
		{
			uint32_t i = (uint32_t)overlaps;
			bool overlapsN[N] = {false};
			for (auto i = 1, j = 0; i <= (uint32_t)Overlaps::_Count; i *= 4, j++)
			{
				overlaps[j] = ((i & (i)) || (i & (i * 2)));
			}
			auto n = 0;
			for (auto i = 0; i < N; ++i)
			{
				if (overlapsN[i])
					n++;
			}
			return n >= NAxis;
		}

		// Expand the AABB to include a point
		void encompass(glm::vec<N, float> point)
		{
			_min = (glm::min)(_min, point);
			_max = (glm::max)(_max, point);
		}

		// Reset bounds to infinite values
		void reset()
		{
			_min = glm::vec<N, float>(std::numeric_limits<float>::infinity());
			_max = glm::vec<N, float>(-std::numeric_limits<float>::infinity());
		}

		static AABB merge(const AABB& a, const AABB& b)
		{
			AABB result;
			result._min = (glm::min)(a._min, b._min);
			result._max = (glm::max)(a._max, b._max);
			return result;
		}

		bool isPointInside(glm::vec<N, float> point)
		{
			for (int i = 0; i < N; ++i)
			{
				if (point[i] < _min[i] || point[i] > _max[i])
				{
					return false;
				}
			}
			return true;
		}

		glm::vec<N, float> getCenter() const { return (_min + _max) * 0.5f; }

		glm::vec<N, float> getHalfExtents() const { return (_max - _min) * 0.5f; }
	};
} // namespace zg::physics

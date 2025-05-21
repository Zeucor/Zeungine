#pragma once
#include <limits>
#include <algorithm>
namespace zg::physics
{
	struct Projection
	{
		float min = (std::numeric_limits<float>::max)();
		float max = -(std::numeric_limits<float>::max)();

		/**
		 * @brief Checks if this projection overlaps with another projection.
		 * @param other The other projection interval.
		 * @return True if they overlap, false otherwise.
		 */
		bool overlaps(const Projection& other) const { return max > other.min && min < other.max; }

		/**
		 * @brief Calculates the amount of overlap between this projection and another.
		 * @param other The other projection interval.
		 * @return The positive overlap amount, or 0 if they don't overlap.
		 */
		float getOverlap(const Projection& other) const
		{
			if (max < min || other.max < other.min)
				return 0.0f;
			return (std::max)(0.0f, (std::min)(max, other.max) - (std::max)(min, other.min));
		}
	};
} // namespace zg::physics

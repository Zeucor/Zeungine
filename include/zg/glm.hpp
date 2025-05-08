#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/closest_point.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>
inline static float ZG_PI = acos(-1);
using uvec = glm::vec<4, uint8_t>;
namespace std
{
	template <size_t N, typename T>
	struct hash<glm::vec<N, T>>
	{
		size_t operator()(const glm::vec<N, T> &vec) const
		{
			auto hash = std::hash<T>{}(vec[0]) << 1;
			for (auto i = 1; i < N; ++i)
			{
				hash ^= std::hash<T>{}(vec[i]) << 3;
			}
			return hash;
		}
	};
}
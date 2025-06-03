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
#include <glm/gtx/hash.hpp>
inline static float ZG_PI = acos(-1);
inline static float ZG_PI_2 = ZG_PI * 2.f;
using uvec = glm::vec<4, uint8_t>;
#define rotate_identity {1, 0, 0, 0}
namespace zg
{
	// Fast max and min assuming no side effects and no NaNs
	// Inline and constexpr for compile-time and runtime optimization
	template <typename T>
	inline constexpr T max(const T& a, const T& b)
	{
		return (a > b) ? a : b;
	}

	template <typename T>
	inline constexpr T min(const T& a, const T& b)
	{
		return (a < b) ? a : b;
	}

	// Vectorized versions for glm::vec3 (element-wise)
	inline glm::vec3 max(const glm::vec3& a, const glm::vec3& b)
	{
		return glm::vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
	}

	inline glm::vec3 min(const glm::vec3& a, const glm::vec3& b)
	{
		return glm::vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
	}
}

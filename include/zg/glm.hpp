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
	template <>
	struct hash<glm::vec2>
	{
		size_t operator()(const glm::vec2 &vec) const
		{
			return std::hash<float>{}(vec.x) ^ std::hash<float>{}(vec.y);
		}
	};
	template <>
	struct hash<glm::vec3>
	{
		size_t operator()(const glm::vec3 &vec) const
		{
			return std::hash<float>{}(vec.x) ^ std::hash<float>{}(vec.y) ^ std::hash<float>{}(vec.z);
		}
	};
	template <>
	struct hash<glm::vec4>
	{
		size_t operator()(const glm::vec4 &vec) const
		{
			return std::hash<float>{}(vec.x) ^ std::hash<float>{}(vec.y) ^ std::hash<float>{}(vec.z) ^ std::hash<float>{}(vec.w);
		}
	};
}
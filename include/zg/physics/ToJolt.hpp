#pragma once
#include <algorithm>
#include <zg/glm.hpp>
#include "Layers.hpp"
template <typename T, typename R>
R ToJolt(T val)
{
	if constexpr (std::is_same_v<T, glm::vec3>)
		if constexpr (std::is_same_v<R, JPH::Vec3>)
			return JPH::Vec3(val.x, val.y, val.z);
	if constexpr (std::is_same_v<T, JPH::Vec3>)
		if constexpr (std::is_same_v<R, glm::vec3>)
            return glm::vec3(val.GetX(), val.GetY(), val.GetZ());
    if constexpr (std::is_same_v<T, glm::quat>)
        if constexpr (std::is_same_v<R, JPH::Quat>)
            return JPH::Quat(val.x, val.y, val.z, val.w);
    if constexpr (std::is_same_v<T, JPH::Quat>)
        if constexpr (std::is_same_v<R, glm::quat>)
            return glm::quat(val.GetW(), val.GetX(), val.GetY(), val.GetZ());
    R r;
    return r;
}

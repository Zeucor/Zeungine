#pragma once
#include <algorithm>
#include <zg/glm.hpp>
#include <zg/physics/Layers.hpp>
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
    if constexpr (std::is_same_v<T, JPH::RMat44>)
        if constexpr (std::is_same_v<R, glm::mat4>)
        {
            glm::mat4 ret;
            ret[0][0] = val(0, 0);
            ret[0][1] = val(1, 0);
            ret[0][2] = val(2, 0);
            ret[0][3] = val(3, 0);
            ret[1][0] = val(0, 1);
            ret[1][1] = val(1, 1);
            ret[1][2] = val(2, 1);
            ret[1][3] = val(3, 1);
            ret[2][0] = val(0, 2);
            ret[2][1] = val(1, 2);
            ret[2][2] = val(2, 2);
            ret[2][3] = val(3, 2);
            ret[3][0] = val(0, 3);
            ret[3][1] = val(1, 3);
            ret[3][2] = val(2, 3);
            ret[3][3] = val(3, 3);
            return ret;
        }
    R r;
    return r;
}

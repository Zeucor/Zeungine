#pragma once
#include <string>
#include <vector>
#include <zg/utilities.hpp>
namespace zg::shaders
{
	using RuntimeConstants = std::vector<std::string>;
	inline static RuntimeConstants common_zg_constants = {
		"NearFarPlanes", "Viewport", "Time", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"
	};
	RuntimeConstants mergeConstants(const std::vector<RuntimeConstants>& constants_z);
}
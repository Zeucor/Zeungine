#pragma once
#include <zg/glm.hpp>
namespace zg::lights
{
	struct PointLight
	{
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 color;
		float intensity;
		float range;
		float nearPlane;
		float farPlane;
		float ambientFactor;
	};
	struct DirectionalLight
	{
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 direction;
		alignas(16) glm::vec3 up;
		alignas(16) glm::vec3 color;
		float intensity;
		float nearPlane;
		float farPlane;
		float ambientFactor;
	};
	struct SpotLight
	{
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 direction;
		alignas(16) glm::vec3 color;
		float intensity;
		float cutoff;
		float outerCutoff;
		float nearPlane;
		float farPlane;
		float ambientFactor;
	};
}
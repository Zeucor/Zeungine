#pragma once
#include "../../../glm.hpp"
namespace anex::modules::gl::lights
{
	struct PointLight
	{
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 color;
		float intensity;
		float range;
	};
	struct DirectionalLight
	{
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 direction;
		alignas(16) glm::vec3 color;
		float intensity;
	};
	struct SpotLight
	{
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 direction;
		alignas(16) glm::vec3 color;
		float intensity;
		float cutoff;
		float outerCutoff;
	};
}
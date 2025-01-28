#pragma once
#include <zg/glm.hpp>
namespace zg
{
	struct ISizable
	{
		glm::vec3 size;
		virtual void setSize(glm::vec3 newSize);
	};
}
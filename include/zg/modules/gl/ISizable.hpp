#pragma once
#include <zg/glm.hpp>
namespace zg::modules::gl
{
	struct ISizable
	{
		glm::vec3 size;
		virtual void setSize(glm::vec3 newSize);
	};
}
#pragma once
#include <anex/glm.hpp>
namespace anex::modules::gl
{
	struct ISizable
	{
		glm::vec3 size;
		virtual void setSize(glm::vec3 newSize);
	};
}
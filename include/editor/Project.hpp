#pragma once
#include <string>
namespace anex::editor
{
	struct Project
	{
		std::string_view name;
		std::string_view directory;
	};
}
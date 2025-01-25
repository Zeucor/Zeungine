#pragma once
#include <string>
namespace zg::editor
{
	struct Project
	{
		std::string_view name;
		std::string_view directory;
	};
}
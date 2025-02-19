#pragma once
#include <string>
using namespace std;
namespace zg::editor
{
	struct Project
	{
		string_view name;
		string_view directory;
	};
}
#pragma once
#include <cstddef>
namespace zg
{
	struct Window;
	struct IEntity
	{
		Window &window;
		size_t ID = 0;
		IEntity(Window &_window);
		virtual ~IEntity() = default;
		virtual void update();
		virtual void render();
	};
}
#pragma once
#include <cstddef>
namespace anex
{
	struct IWindow;
	struct IEntity
	{
		IWindow &window;
		size_t ID = 0;
		IEntity(IWindow &_window);
		virtual ~IEntity() = default;
		virtual void update();
		virtual void render();
  };
}
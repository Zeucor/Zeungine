#pragma once
namespace anex
{
	struct IWindow;
	struct IEntity
	{
		IWindow &window;
		size_t ID = 0;
		IEntity(IWindow &window);
		virtual ~IEntity() = default;
		virtual void update();
		virtual void render();
  };
}
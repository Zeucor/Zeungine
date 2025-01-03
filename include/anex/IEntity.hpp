#pragma once
namespace anex
{
	struct IWindow;
	struct IEntity
	{
		IWindow &window;
		IEntity(IWindow &window);
		virtual ~IEntity() = default;
		virtual void render();
  };
}
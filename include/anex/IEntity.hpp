#pragma once
namespace anex
{
	struct IGame;
	struct IEntity
	{
		IGame &game;
		IEntity(IGame &game);
		virtual ~IEntity() = default;
		virtual void render() = 0;
  };
}
#pragma once
#include <unordered_map>
#include <memory>
#include "./IEntity.hpp"
namespace anex
{
	struct IGame;
	struct IScene
	{
		IGame &game;
		std::unordered_map<unsigned int, std::shared_ptr<IEntity>> entities;
		unsigned int entitiesCount;
		IScene(IGame &game);
		virtual ~IScene() = default;
		unsigned int addEntity(const std::shared_ptr<IEntity> &entity);
		void removeEntity(const unsigned int &id);
		void render();
	};
}
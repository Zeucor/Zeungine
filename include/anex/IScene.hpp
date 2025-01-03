#pragma once
#include <map>
#include <memory>
#include "./IEntity.hpp"
namespace anex
{
	struct IWindow;
	struct IScene
	{
		IWindow &window;
		std::map<unsigned int, std::shared_ptr<IEntity>> entities;
		unsigned int entitiesCount;
		IScene(IWindow &window);
		virtual ~IScene() = default;
		unsigned int addEntity(const std::shared_ptr<IEntity> &entity);
		void removeEntity(const unsigned int &id);
		virtual void preRender() = 0;
		virtual void render();
		virtual void entityPreRender(IEntity &entity);
	};
}
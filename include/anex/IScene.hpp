#pragma once
#include <map>
#include <memory>
#include <vector>
#include <glm/vec2.hpp>

namespace anex
{
	struct IWindow;
	struct IEntity;
	struct IScene
	{
		IWindow &window;
		std::map<size_t, std::shared_ptr<IEntity>> entities;
		size_t entitiesCount = 0;
		IScene(IWindow &window);
		virtual ~IScene() = default;
		size_t addEntity(const std::shared_ptr<IEntity> &entity);
		void removeEntity(const size_t &id);
		virtual void update();
		virtual void preRender() = 0;
		virtual void render();
		virtual void entityPreRender(IEntity &entity);
		virtual void postAddEntity(const std::shared_ptr<IEntity> &entity, const std::vector<size_t> &entityIDs);
		virtual void resize(const glm::vec2 &newSize);
	};
}

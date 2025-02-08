#pragma once
#include <map>
#include <memory>
#include <vector>
#include <glm/vec2.hpp>

namespace zg
{
	struct Window;
	struct IEntity;
	struct IScene
	{
		Window &window;
		std::map<size_t, std::shared_ptr<IEntity>> entities;
		size_t entitiesCount = 0;
		IScene(Window &_window);
		virtual ~IScene() = default;
		size_t addEntity(const std::shared_ptr<IEntity> &entity, bool callOnEntityAdded = true);
		void removeEntity(const size_t &id);
		virtual void update();
		virtual void preRender() = 0;
		virtual void render();
		virtual void entityPreRender(IEntity &entity);
		virtual void postAddEntity(const std::shared_ptr<IEntity> &entity, const std::vector<size_t> &entityIDs);
		virtual void preRemoveEntity(const std::shared_ptr<IEntity> &entity, const std::vector<size_t> &entityIDs);
		virtual void resize(glm::vec2 newSize);
	};
}
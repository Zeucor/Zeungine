#include <zg/IScene.hpp>
#include <zg/IEntity.hpp>
#include <zg/IWindow.hpp>
using namespace zg;
IScene::IScene(IWindow& _window):
	window(_window)
{
};
size_t IScene::addEntity(const std::shared_ptr<IEntity>& entity, bool callOnEntityAdded)
{
	auto id = ++entitiesCount;
	entity->ID = id;
	entities[id] = entity;
	postAddEntity(entity, {id});
	if (callOnEntityAdded && window.onEntityAdded)
		window.onEntityAdded(entity);
	return id;
};
void IScene::removeEntity(const size_t& id)
{
	auto entityIter = entities.find(id);
	if (entityIter != entities.end())
	{
		preRemoveEntity(entityIter->second, {id});
		entityIter->second->ID = 0;
		entities.erase(entityIter);
	}
};
void IScene::update()
{
	auto it = entities.begin();
	auto end = entities.end();
	for (; it != end; it++)
	{
		it->second->update();
	}
};
void IScene::render()
{
	preRender();
	auto it = entities.begin();
	auto end = entities.end();
	for (; it != end; it++)
	{
		it->second->render();
	}
};
void IScene::entityPreRender(IEntity &entity){};
void IScene::postAddEntity(const std::shared_ptr<IEntity>& entity, const std::vector<size_t> &entityIDs){};
void IScene::preRemoveEntity(const std::shared_ptr<IEntity>& entity, const std::vector<size_t> &entityIDs){};
void IScene::resize(glm::vec2 newSize){};
#include <anex/IScene.hpp>
using namespace anex;
IScene::IScene(IWindow& window):
	window(window)
{
};
unsigned int IScene::addEntity(const std::shared_ptr<IEntity>& entity)
{
	auto id = ++entitiesCount;
	entities[id] = entity;
	return id;
};
void IScene::removeEntity(const unsigned int& id)
{
	auto entityIter = entities.find(id);
	if (entityIter != entities.end())
	{
		entities.erase(entityIter);
	}
};
void IScene::render()
{
	for (auto& entityPair : entities)
	{
		auto& entity = entityPair.second;
		entity->render();
	}
}
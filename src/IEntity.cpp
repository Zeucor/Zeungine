#include <anex/IEntity.hpp>
using namespace anex;
IEntity::IEntity(IWindow &window):
	window(window)
{};
void IEntity::update(){};
void IEntity::render(){};

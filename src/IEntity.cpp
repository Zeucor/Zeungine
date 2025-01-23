#include <anex/IEntity.hpp>
using namespace anex;
IEntity::IEntity(IWindow &_window):
	window(_window)
{};
void IEntity::update(){};
void IEntity::render(){};

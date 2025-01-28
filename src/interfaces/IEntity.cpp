#include <zg/interfaces/IEntity.hpp>
using namespace zg;
IEntity::IEntity(IWindow &_window):
	window(_window)
{};
void IEntity::update(){};
void IEntity::render(){};

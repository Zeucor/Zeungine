#include <zg/components/entities/Collider.hpp>
#include <zg/Entity.hpp>
using namespace zg::components::entities;
Collider::Collider(const ColliderInfo &info) :
		IEntityComponent("Collider"), info(info), transform(&info.entity.getModelMatrix())
{
}
void Collider::onAttached() { return; }
void Collider::onUpdate() { return; }
void Collider::onDetached() { return; }

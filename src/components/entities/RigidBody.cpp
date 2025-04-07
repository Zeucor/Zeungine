#include <zg/Entity.hpp>
#include <zg/components/entities/RigidBody.hpp>
using namespace zg::components::entities;
RigidBody::RigidBody(const RigidBodyInfo& info) :
		IEntityComponent("RigidBody"), info(info), transform(&info.entity.getModelMatrix())
{
}
void RigidBody::onAttached() {}
void RigidBody::onUpdate() {}
void RigidBody::onDetached() {}

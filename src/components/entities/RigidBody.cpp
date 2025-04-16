#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
using namespace zg::components::entities;
RigidBody::RigidBody(const RigidBodyInfo& info) :
		IEntityComponent("RigidBody"), info(info), transform(&info.entity.getModelMatrix()),
		position(&info.entity.position), rotation(&info.entity.rotation)
{
}
void RigidBody::addCollider(Collider* collider)
{
	colliders.push_back(collider);
	inverseInertiaTensorBody = collider->getColliderInfo().shapeData->calculateInverseInertiaBody(getMass());
}
void RigidBody::removeCollider(Collider* collider)
{
	for (auto iter = colliders.begin(), end = colliders.end(); iter != end; iter++)
	{
		if (*iter == collider)
		{
			colliders.erase(iter);
			break;
		}
	}
}
void RigidBody::onAttached()
{
	physicsScene = dynamic_cast<scenes::PhysicsScene*>(info.entity.scene.getComponentByName("PhysicsScene").get());
	if (!physicsScene)
	{
		throw std::runtime_error("RigidBody component must not be added to an entity before the scene the entity is in has "
														 "an attached PhysicsScene component");
	}

	// TODO: Calculate inverse inertia tensor based on mass and collider shapes/offsets

	physicsScene->registerRigidBody(this);
	std::cout << "RigidBody attached." << std::endl;
}
void RigidBody::onUpdate()
{
	// This is the engine's per-frame update, not the fixed physics step.
	// Could be used for:
	// - Applying continuous user forces (though often better just before physics update)
	// - Updating kinematic target positions/rotations before the physics step reads them.
}
void RigidBody::onDetached()
{
	// Unregister from the physics system
	if (physicsScene)
	{
		physicsScene->unregisterRigidBody(this);
	}
	physicsScene = nullptr;
	transform = nullptr; // Clear pointer
	std::cout << "RigidBody detached." << std::endl;
}
void RigidBody::applyForce(glm::vec3 localForce, glm::vec3 worldPoint, float dt)
{
	if (!isDynamic())
	{
		return;
	}
	// glm::quat orientation = getOrientation(); 
	// glm::vec3 worldForce = orientation * localForce;
	forceAccumulator += localForce;
	glm::vec3 centerOfMass = getPosition();
	glm::vec3 pointRelative = worldPoint - centerOfMass;
	torqueAccumulator += glm::cross(pointRelative, localForce);
	setSleeping(false);
}
void RigidBody::applyLocalForceToCenter(glm::vec3 localForce, float dt)
{
	if (!isDynamic())
		return;
	glm::quat orientation = getOrientation(); // Assumes getOrientation() returns the world orientation quaternion
	glm::vec3 worldForce = orientation * localForce; // Rotate local force vector into world space
	forceAccumulator += worldForce;
	// linearAcceleration = forceAccumulator / info.mass;
	// if (dt)
	// {
	// 	linearVelocity = linearVelocity + linearAcceleration * dt;
	// }
	setSleeping(false);
}
void RigidBody::applyForceToCenter(glm::vec3 force, float dt)
{
	if (!isDynamic())
		return;
	auto& rotationAxisDegrees = this->info.entity.rotation;
	forceAccumulator += force;
	// linearAcceleration = forceAccumulator / info.mass;
	// if (dt)
	// {
	// 	linearVelocity = linearVelocity + linearAcceleration * dt;
	// }
	// No torque generated when applied to the center of mass
	setSleeping(false);
	return;
}
void RigidBody::applyTorque(glm::vec3 torque, float dt)
{
	if (!isDynamic())
		return;
	torqueAccumulator += torque;
	// angularAcceleration = torqueAccumulator / info.mass;
	// if (dt)
	// {
	// 	angularVelocity = angularVelocity + (angularAcceleration * dt);
	// }
	setSleeping(false);
}
void RigidBody::clearForces()
{
	forceAccumulator = glm::vec3(0);
	torqueAccumulator = glm::vec3(0);
}

const glm::vec3& RigidBody::getPosition() const { return *position; }

const glm::quat& RigidBody::getOrientation() const { return *rotation; }

// void RigidBody::setPosition(glm::vec3 pos)
// {
// 	if (!position)
// 		return;
// 	*position = pos;
// 	info.entity.updateNonce--;
// 	info.entity.getModelMatrix();
// }

// void RigidBody::setRotation(glm::vec3 rot)
// {
// 	if (!rotation)
// 		return;
// 	*rotation += rot;
// 	info.entity.updateNonce--;
// 	info.entity.getModelMatrix();
// }

void RigidBody::translate(glm::vec3 deltaPos)
{
	// More efficient than setPosition(getPosition() + deltaPos)
	*position += deltaPos;
	if (std::isnan(position->x) || std::isnan(position->y) || std::isnan(position->z))
	{
		return;
	}
	info.entity.updateNonce--;
	info.entity.getModelMatrix();
}
bool RigidBody::isStatic() const { return info.bodyType == BodyType::Static; }
bool RigidBody::isKinematic() const { return info.bodyType == BodyType::Kinematic; }
bool RigidBody::isDynamic() const { return info.bodyType == BodyType::Dynamic; }
glm::mat3 RigidBody::getInverseInertiaTensorWorld() const
{
	if (isStatic())
	{
		return glm::mat3(0.0f);
	}
	glm::mat3 R = glm::toMat3(getOrientation());
	const glm::mat3& I_body_inv = inverseInertiaTensorBody;
	glm::mat3 I_world_inv = R * I_body_inv * glm::transpose(R);
	return I_world_inv;
}
void RigidBody::update(float dt, bool _clearForces)
{
	if (!sleeping && isDynamic())
	{
		auto transforms = scenes::PhysicsScene::getTransformsAtTime(this, dt);
		*position = transforms.first;
		*rotation = transforms.second;
		// auto _linear_damping = (std::max)(0.0f, (std::min)(1.0f, info.linearDamping));
		// auto linear_multiplier = std::pow(_linear_damping, dt);
		// linearVelocity = linearVelocity * linear_multiplier;
		// glm::vec3 linearAcceleration = forceAccumulator * getInverseMass();
		// linearVelocity += linearAcceleration * dt;
		// *position += linearVelocity * dt;
		// auto _angular_damping = (std::max)(0.0f, (std::min)(1.0f, info.angularDamping));
		// auto angular_multiplier = std::pow(_angular_damping, dt);
		// angularVelocity = angularVelocity * angular_multiplier;
		// float angle = glm::length(angularVelocity) * dt;
		// if (angle > 0)
		// {
		// 	glm::vec3 axis = glm::normalize(angularVelocity);
		// 	glm::quat rotationDelta = glm::angleAxis(angle, axis);
		// 	*rotation *= rotationDelta;
		// 	*rotation = glm::normalize(*rotation);
		// }
		// glm::vec3 angularAcceleration = getInverseInertiaTensorWorld() * torqueAccumulator;
		// angularVelocity += angularAcceleration * dt;

		info.entity.updateNonce--;
		info.entity.getModelMatrix();
		for (auto& collider : getColliders())
		{
			collider->getWorldAABB();
		}
		if (_clearForces)
		{
			clearForces();
		}
	}
	return;
}
bool RigidBody::isTouching(RigidBody& rigidBody, physics::CollisionMannifold*& mannifoldPointer)
{
	auto iter = activeRigidBodyMannifolds.find(&rigidBody);
	if (iter == activeRigidBodyMannifolds.end())
	{
		return false;
	}
	mannifoldPointer = &iter->second;
	return true;
}
void RigidBody::addActiveMannifold(const physics::CollisionMannifold& mannifold)
{
	static auto isCollider = [](RigidBody& rb, Collider* c)
	{
		auto cs = rb.getColliders();
		for (auto& cv : cs)
			if (cv == c)
				return true;
		return false;
	};
	auto rigidBodyPointer = isCollider(*this, mannifold.colliderA) ? mannifold.colliderB->getOwnerRigidBody()
																																 : mannifold.colliderA->getOwnerRigidBody();
	activeRigidBodyMannifolds[rigidBodyPointer] = mannifold;
}
void RigidBody::clearActiveMannifolds() { activeRigidBodyMannifolds.clear(); }

#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
using namespace zg::components::entities;
RigidBody::RigidBody(const RigidBodyInfo& info) :
		IEntityComponent("RigidBody"), info(info), position(&info.entity.position), rotation(&info.entity.rotation),
		joltBodyID()
{
}
void RigidBody::addCollider(Collider* collider)
{
	if (!collider)
		return;
	// Avoid adding duplicates
	for (const auto* existing : colliders)
	{
		if (existing == collider)
			return;
	}

	colliders.push_back(collider);
	recreateJoltBody();
}
void RigidBody::removeCollider(Collider* collider)
{
	bool changed = false;
	for (auto iter = colliders.begin(), end = colliders.end(); iter != end; ++iter)
	{
		if (*iter == collider)
		{
			colliders.erase(iter);
			changed = true;
			break;
		}
	}

	// If the Jolt body exists and a collider was actually removed, recreate the body.
	if (changed && joltBodyInterface && !joltBodyID.IsInvalid())
	{
		std::cout << "RigidBody: Collider removed. Recreating Jolt body for Entity [" << info.entity.ID << "]."
							<< std::endl;
		recreateJoltBody();
	}
}
void RigidBody::recreateJoltBody()
{
	if (!joltBodyInterface)
	{
		std::cerr << "RigidBody ERROR: Cannot recreate Jolt body, Jolt system not available for Entity [" << info.entity.ID
							<< "]." << std::endl;
		return;
	}

	// --- Remove existing body if it exists ---
	if (!joltBodyID.IsInvalid())
	{
		joltBodyInterface->RemoveBody(joltBodyID);
		joltBodyInterface->DestroyBody(joltBodyID); // Destroy after removing
		joltBodyID = JPH::BodyID();
	}

	// --- Check if there are any colliders ---
	if (colliders.empty())
	{
		// std::cout << "RigidBody NOTE: No colliders found for Entity [" << info.entity.ID << "], Jolt body not created."
		// << std::endl;
		return; // Cannot create a body without a shape
	}

	// --- Create Jolt Shape ---
	JPH::ShapeRefC finalShape = nullptr;

	if (colliders.size() == 1)
	{
		// Single collider case
		Collider* col = colliders[0];
		auto& colInfo = col->getColliderInfo();
		JPH::ShapeRefC baseShape = colInfo.shapeData->createJoltShape();
		if (!baseShape)
		{
			std::cerr << "RigidBody ERROR: Failed to create base Jolt shape for Entity [" << info.entity.ID << "]."
								<< std::endl;
			return;
		}

		// Apply offset and rotation if necessary
		bool hasOffset = glm::length2(colInfo.offset) > 1e-6f;
		// Check if rotation is not identity
		bool hasRotation = glm::abs(glm::dot(colInfo.rotationOffset, glm::quat(1.f, 0.f, 0.f, 0.f))) < (1.f - 1e-6f);

		if (hasOffset || hasRotation)
		{
			finalShape = new RotatedTranslatedShape(ToJolt<glm::vec3, JPH::Vec3>(colInfo.offset),
																							ToJolt<glm::quat, JPH::Quat>(colInfo.rotationOffset), baseShape);
		}
		else
		{
			finalShape = baseShape;
		}
	}
	else
	{
		throw std::runtime_error("Multiple collider Bodies not yet supported");
		// Multiple colliders: Create a CompoundShape
		// JPH::StaticCompoundShapeSettings compoundSettings;
		// for (const Collider* col : colliders)
		// {
		// 	const auto& colInfo = col->getColliderInfo();
		// 	JPH::ShapeRefC baseShape = colInfo.shapeData->createJoltShape();
		// 	if (baseShape)
		// 	{
		// 		compoundSettings.AddShape(ToJolt<glm::vec3, JPH::Quat>(colInfo.offset),
		// 															ToJolt<glm::quat, JPH::Quat>(colInfo.rotationOffset), baseShape, 0);
		// 		// Note: Material/Sensor properties might need to be handled per sub-shape if Jolt supports it,
		// 		// otherwise the compound shape usually gets single properties. Jolt allows sub-shape user data.
		// 	}
		// 	else
		// 	{
		// 		std::cerr << "RigidBody WARNING: Failed to create sub-shape for compound shape on Entity [" << info.entity.ID
		// 							<< "]." << std::endl;
		// 	}
		// }
		// auto result = compoundSettings.Create();
		// if (result.HasError())
		// {
		// 	std::cerr << "RigidBody ERROR: Failed to create Jolt compound shape for Entity [" << info.entity.ID
		// 						<< "]: " << result.GetError().c_str() << std::endl;
		// 	return;
		// }
		// finalShape = result.Get();
	}

	if (!finalShape)
	{
		std::cerr << "RigidBody ERROR: Final Jolt shape is null for Entity [" << info.entity.ID << "]." << std::endl;
		return;
	}

	// // --- Create BodyCreationSettings ---
	BodyCreationSettings bodySettings;
	bodySettings.SetShape(finalShape);
	bodySettings.mPosition = ToJolt<glm::vec3, JPH::Vec3>(*position);
	bodySettings.mRotation = ToJolt<glm::quat, JPH::Quat>(*rotation);
	bodySettings.mUserData = info.entity.ID; // Store entity ID

	// // Determine MotionType and ObjectLayer (Using simple layers for now)
	// // TODO: Get layers from PhysicsScene or project settings
	ObjectLayer objectLayer;
	switch (info.bodyType)
	{
	case BodyType::Static:
		bodySettings.mMotionType = JPH::EMotionType::Static;
		objectLayer = Layers::NON_MOVING; // Assumes Layers namespace exists (from JoltPhysicsSystem example)
		break;
	case BodyType::Kinematic:
		bodySettings.mMotionType = JPH::EMotionType::Kinematic;
		objectLayer = Layers::MOVING;
		break;
	case BodyType::Dynamic:
	default:
		bodySettings.mMotionType = JPH::EMotionType::Dynamic;
		objectLayer = Layers::MOVING;
		break;
	}
	bodySettings.mObjectLayer = objectLayer;

	// Apply properties from the first collider (or average/dominant if multiple?)
	// Jolt applies friction/restitution per body pair based on a combine function.
	// Setting them on the body provides the base values.
	if (!colliders.empty())
	{
		const auto& mat = colliders[0]->getPhysicsMaterial(); // Use first collider's material
		bodySettings.mFriction = mat.friction;
		bodySettings.mRestitution = mat.restitution;
		// Sensor property - affects the whole body in Jolt unless using specific contact listener logic
		bodySettings.mIsSensor = colliders[0]->getIsSensor();
	}

	bodySettings.mLinearDamping = info.linearDamping;
	bodySettings.mAngularDamping = info.angularDamping;
	bodySettings.mGravityFactor = info.useGravity ? 1.0f : 0.0f;
	bodySettings.mAllowSleeping = true; // Default

	// Mass properties for dynamic bodies
	if (info.bodyType == BodyType::Dynamic)
	{
		// MassProperties calculated from shape needs volume > 0
		if (info.mass <= 0.0f)
		{
			std::cerr << "RigidBody WARNING: Dynamic body has zero or negative mass on Entity [" << info.entity.ID
								<< "]. Setting to 1.0." << std::endl;
			info.mass = 1.0f;
		}
		bodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		bodySettings.mMassPropertiesOverride.mMass = info.mass;
		// Jolt calculates inertia based on shape and mass.
		// Can override inertia tensor here if needed:
		// bodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateMassAndInertia; // If shape defines
		// density bodySettings.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;
		// bodySettings.mMassPropertiesOverride.mInertia = JPH::Mat44::sRotation(...) * inertiaTensor * ...;
	}

	// --- Create and Add Body ---
	body = joltBodyInterface->CreateBody(bodySettings);
	if (!body)
	{
		std::cerr << "RigidBody ERROR: Failed to create Jolt body for Entity [" << info.entity.ID << "]." << std::endl;
		return;
	}
	joltBodyID = body->GetID(); // Store the new ID

	// // Add the body to the simulation (activate it)
	joltBodyInterface->AddBody(joltBodyID, EActivation::Activate);

	physicsScene->registerRigidBody(this);
	// std::cout << "RigidBody: Successfully created/recreated Jolt body for Entity [" << info.entity.ID << "] with ID "
	// << joltBodyID.GetIndex() << std::endl;
};
void RigidBody::onAttached()
{
	physicsScene = dynamic_cast<scenes::PhysicsScene*>(info.entity.scene.getComponentByName("PhysicsScene").get());
	if (!physicsScene)
	{
		throw std::runtime_error("RigidBody component must not be added to an entity before the scene the entity is in has "
														 "an attached PhysicsScene component");
	}
	joltBodyInterface = &physicsScene->GetBodyInterface();
	std::cout << "RigidBody attached." << std::endl;
}
void RigidBody::onUpdate() {}
void RigidBody::onDetached()
{
	physicsScene->unregisterRigidBody(this);
	physicsScene = 0;
}
void RigidBody::applyForce(glm::vec3 worldForce, glm::vec3 worldPoint)
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	bodyInterface.AddForce(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(worldForce),
												 ToJolt<glm::vec3, JPH::Vec3>(worldPoint));
	bodyInterface.ActivateBody(joltBodyID);
}
void RigidBody::applyLocalForceToCenter(glm::vec3 localForce)
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	auto bodyRotation = bodyInterface.GetRotation(joltBodyID);
	auto worldForce = bodyRotation * ToJolt<glm::vec3, JPH::Vec3>(localForce);
	bodyInterface.AddForce(joltBodyID, worldForce);
	bodyInterface.ActivateBody(joltBodyID);
}
void RigidBody::applyForceToCenter(glm::vec3 force)
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	bodyInterface.AddForce(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(force));
	bodyInterface.ActivateBody(joltBodyID);
}
void RigidBody::applyTorque(glm::vec3 torque)
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	bodyInterface.AddTorque(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(torque));
	bodyInterface.ActivateBody(joltBodyID);
}
void RigidBody::clearForces()
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	bodyInterface.SetLinearVelocity(joltBodyID, JPH::Vec3::sZero());
	bodyInterface.SetAngularVelocity(joltBodyID, JPH::Vec3::sZero());
}
const glm::vec3& RigidBody::getPosition() const { return *position; }
void RigidBody::setPosition(glm::vec3 newPosition)
{
    physicsScene->GetBodyInterface().SetPosition(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(newPosition), JPH::EActivation::Activate);
}
const glm::quat& RigidBody::getOrientation() const { return *rotation; }
void RigidBody::setOrientation(glm::quat newOrientation)
{
    physicsScene->GetBodyInterface().SetRotation(joltBodyID, ToJolt<glm::quat, JPH::Quat>(newOrientation), JPH::EActivation::Activate);
}
const glm::vec3 RigidBody::getLinearVelocity() const
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	return ToJolt<JPH::Vec3, glm::vec3>(bodyInterface.GetLinearVelocity(joltBodyID));
}
RigidBodyInfo& RigidBody::getInfo() { return info; }
const RigidBodyInfo& RigidBody::getInfo() const { return info; }
void RigidBody::setLinearVelocityX(float va)
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	auto currentVel = bodyInterface.GetLinearVelocity(joltBodyID);
	bodyInterface.SetLinearVelocity(joltBodyID, JPH::Vec3(va, currentVel.GetY(), currentVel.GetZ()));
	bodyInterface.ActivateBody(joltBodyID);
}
void RigidBody::setLinearVelocityY(float va)
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	auto currentVel = bodyInterface.GetLinearVelocity(joltBodyID);
	bodyInterface.SetLinearVelocity(joltBodyID, JPH::Vec3(currentVel.GetX(), va, currentVel.GetZ()));
	bodyInterface.ActivateBody(joltBodyID);
}
void RigidBody::setLinearVelocityZ(float va)
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	auto currentVel = bodyInterface.GetLinearVelocity(joltBodyID);
	bodyInterface.SetLinearVelocity(joltBodyID, JPH::Vec3(currentVel.GetX(), currentVel.GetY(), va));
	bodyInterface.ActivateBody(joltBodyID);
}
void RigidBody::setAngularVelocityX(float va)
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	auto currentVel = bodyInterface.GetAngularVelocity(joltBodyID);
	bodyInterface.SetAngularVelocity(joltBodyID, JPH::Vec3(va, currentVel.GetY(), currentVel.GetZ()));
	bodyInterface.ActivateBody(joltBodyID);
}
void RigidBody::setAngularVelocityY(float va)
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	auto currentVel = bodyInterface.GetAngularVelocity(joltBodyID);
	bodyInterface.SetAngularVelocity(joltBodyID, JPH::Vec3(currentVel.GetX(), va, currentVel.GetZ()));
	bodyInterface.ActivateBody(joltBodyID);
}
void RigidBody::setAngularVelocityZ(float va)
{
	auto& bodyInterface = physicsScene->GetBodyInterface();
	auto currentVel = bodyInterface.GetAngularVelocity(joltBodyID);
	bodyInterface.SetAngularVelocity(joltBodyID, JPH::Vec3(currentVel.GetX(), currentVel.GetY(), va));
	bodyInterface.ActivateBody(joltBodyID);
}
void RigidBody::setLinearVelocity(glm::vec3 newLinearVelocity)
{
	JPH::BodyInterface& bodyInterface = physicsScene->GetBodyInterface();
	bodyInterface.SetLinearVelocity(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(newLinearVelocity));
	bodyInterface.ActivateBody(joltBodyID);
}
void RigidBody::setAngularVelocity(glm::vec3 newAngularVelocity)
{
	JPH::BodyInterface& bodyInterface = physicsScene->GetBodyInterface();
	bodyInterface.SetAngularVelocity(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(newAngularVelocity));
	bodyInterface.ActivateBody(joltBodyID);
}
bool RigidBody::isSleeping() { return sleeping; }
void RigidBody::setSleeping(bool newSleeping)
{
	if (sleeping != newSleeping)
		sleeping = newSleeping;
}
JPH::BodyID RigidBody::getJoltBodyID() const { return joltBodyID; }
glm::vec3 RigidBody::getAngularVelocity() const { throw ""; }
float RigidBody::getMass() const { return info.mass; }
void RigidBody::setMass(float newMass) { info.mass = newMass; }
float RigidBody::getInverseMass() const
{
	return (info.bodyType == BodyType::Dynamic && info.mass > 0.0f) ? 1.0f / info.mass : 0.0f;
}
const std::vector<Collider*>& RigidBody::getColliders() const { return colliders; }
bool RigidBody::getUseGravity() const { return info.useGravity; }
glm::vec3 RigidBody::getFreezeRotationAxes() const { return info.freezeRotationAxes; }
glm::vec3 RigidBody::getFreezeVelocityAxes() const { return info.freezeVelocityAxes; }
glm::vec3 RigidBody::getForceAccumulator() const { throw ""; }
glm::vec3 RigidBody::getTorqueAccumulator() const { throw ""; }
float RigidBody::getLinearDamping() { return info.linearDamping; }
float RigidBody::getAngularDamping() { return info.angularDamping; }
void RigidBody::translate(glm::vec3 deltaPos)
{
	// translate Jolt body
	// update *position and *rotation
}
bool RigidBody::isStatic() const { return info.bodyType == BodyType::Static; }
bool RigidBody::isKinematic() const { return info.bodyType == BodyType::Kinematic; }
bool RigidBody::isDynamic() const { return info.bodyType == BodyType::Dynamic; }
glm::vec3 RigidBody::getCenterAtTime(float t) const { return *position + getLinearVelocity() * t; }
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

#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
#include <zg/Registry.hpp>
using namespace zg::components::entities;
zg::components::entities::EntityComponentCreateInfo zg::components::entities::RigidBodyFactory(const RigidBodyInfo& info)
{
	zg::components::entities::EntityComponentCreateInfo createInfo{
		.name = "RigidBody",
		.onAttachedFunction = [&, info](auto& component)
		{
			auto& entity = Registry::getEntity(component.hostIndexStack);
			auto& physicsScene = component.template make<zg::components::scenes::SceneComponent*>("PhysicsScene", nullptr);
			try
			{
				physicsScene = &entity.scene.getComponentByName("PhysicsScene");
			}
			catch(...)
			{
				throw std::runtime_error("RigidBody component must not be added to an entity before the scene the entity is in has "
																	"an attached PhysicsScene component");
			}
			component.template make<RigidBodyInfo*>("Info", new RigidBodyInfo{info});
			component.template make<glm::vec3*>("Position", &entity.position);
			component.template make<glm::quat*>("Rotation", &entity.rotation);
			component.template make<JPH::BodyID>("BodyID");
			component.template make<JPH::BodyInterface*>("BodyInterface", physicsScene->getData<JPH::BodyInterface*>("BodyInterface"));
			component.template make<JPH::Body*>("Body");
			component.template make<std::vector<components::entities::EntityComponent*>>("Colliders");
			component.template make<std::unordered_map<components::entities::EntityComponent*, physics::CollisionManifold>>("ActiveRigidBodyManifolds");
			component.template make<std::mutex*>("Mutex", new std::mutex());
		},
		.onDetachedFunction = [](auto& component)
		{

		},
		.onUpdateFunction = [](auto& component)
		{

		},
		.getDataFunctions = {
			{"recreateJoltBody", [](auto& component)->std::any&
			{
				auto& entity = Registry::getEntity(component.hostIndexStack);
				auto& physicsScene = *component.template getData<zg::components::scenes::SceneComponent*>("PhysicsScene");
				auto& info = *component.template make<RigidBodyInfo*>("Info");
				auto& position = *component.template make<glm::vec3*>("Position");
				auto& rotation = *component.template make<glm::quat*>("Rotation");
				auto& bodyIDAny = component.getDataReturnAny("BodyID");
				auto& bodyID = std::any_cast<JPH::BodyID&>(bodyIDAny);
				auto& body = component.template getData<JPH::Body*>("Body");
				auto& joltBodyInterfacePointer = component.template getData<JPH::BodyInterface*>("BodyInterface");
				if (!joltBodyInterfacePointer)
				{
					std::cerr << "RigidBody ERROR: Cannot recreate Jolt body, Jolt system not available for Entity [" << entity.ID
										<< "]." << std::endl;
					return bodyIDAny;
				}
				auto& joltBodyInterface = *joltBodyInterfacePointer;
				// --- Remove existing body if it exists ---
				if (!bodyID.IsInvalid())
				{
					joltBodyInterface.RemoveBody(bodyID);
					joltBodyInterface.DestroyBody(bodyID); // Destroy after removing
					bodyID = JPH::BodyID();
				}

				auto& colliders = component.template getData<std::vector<components::entities::EntityComponent*>>("Colliders");
				
				// --- Check if there are any colliders ---
				if (colliders.empty())
				{
					return bodyIDAny; // Cannot create a body without a shape
				}
				
				// --- Create Jolt Shape ---
				JPH::ShapeRefC finalShape = nullptr;
				
				if (colliders.size() == 1)
				{
					// Single collider case
					auto col = colliders[0];
					auto& colInfo = *col->getData<ColliderInfo*>("Info");
					auto baseShape = colInfo.shapeData->createJoltShape();
					if (!baseShape)
					{
						std::cerr << "RigidBody ERROR: Failed to create base Jolt shape for Entity [" << entity.ID << "]."
											<< std::endl;
						return bodyIDAny;
					}
			
					// Apply offset and rotation if necessary
					bool hasOffset = glm::length2(colInfo.offset) > 1e-6f;
					// Check if rotation is not identity
					bool hasRotation = glm::abs(glm::dot(colInfo.rotationOffset, glm::quat(1.f, 0.f, 0.f, 0.f))) < (1.f - 1e-6f);
			
					if (hasOffset || hasRotation)
					{
						finalShape = new JPH::RotatedTranslatedShape(ToJolt<glm::vec3, JPH::Vec3>(colInfo.offset),
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
					// 		std::cerr << "RigidBody WARNING: Failed to create sub-shape for compound shape on Entity [" << entity.ID
					// 							<< "]." << std::endl;
					// 	}
					// }
					// auto result = compoundSettings.Create();
					// if (result.HasError())
					// {
					// 	std::cerr << "RigidBody ERROR: Failed to create Jolt compound shape for Entity [" << entity.ID
					// 						<< "]: " << result.GetError().c_str() << std::endl;
					// 	return bodyIDAny;
					// }
					// finalShape = result.Get();
				}
				
				if (!finalShape)
				{
					std::cerr << "RigidBody ERROR: Final Jolt shape is null for Entity [" << entity.ID << "]." << std::endl;
					return bodyIDAny;
				}
			
				// // --- Create BodyCreationSettings ---
				JPH::BodyCreationSettings bodySettings;
				bodySettings.SetShape(finalShape);
				bodySettings.mPosition = ToJolt<glm::vec3, JPH::Vec3>(position);
				bodySettings.mRotation = ToJolt<glm::quat, JPH::Quat>(rotation);
				bodySettings.mUserData = entity.ID; // Store entity ID
				
				// // Determine MotionType and ObjectLayer (Using simple layers for now)
				// // TODO: Get layers from PhysicsScene or project settings
				JPH::ObjectLayer objectLayer;
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
					auto& collider0 = *colliders[0];
					const auto& mat = collider0.getData<PhysicsMaterial>("PhysicsMaterial"); // Use first collider's material
					bodySettings.mFriction = mat.friction;
					bodySettings.mRestitution = mat.restitution;
					// Sensor property - affects the whole body in Jolt unless using specific contact listener logic
					bodySettings.mIsSensor = collider0.getData<bool>("IsSensor");
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
						std::cerr << "RigidBody WARNING: Dynamic body has zero or negative mass on Entity [" << entity.ID
											<< "]. Setting to 1.0." << std::endl;
						info.mass = 1.0f;
					}
					bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
					bodySettings.mMassPropertiesOverride.mMass = info.mass;
					// Jolt calculates inertia based on shape and mass.
					// Can override inertia tensor here if needed:
					// bodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateMassAndInertia; // If shape defines
					// density bodySettings.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;
					// bodySettings.mMassPropertiesOverride.mInertia = JPH::Mat44::sRotation(...) * inertiaTensor * ...;
				}
				
				// bodySettings.mAllowedDOFs = JPH::EAllowedDOFs::Plane2D;
				bodySettings.mAllowedDOFs = JPH::EAllowedDOFs::None;
				if (!info.freezeRotationAxes.x)
				{
					bodySettings.mAllowedDOFs |= JPH::EAllowedDOFs::RotationX;
				}
				if (!info.freezeRotationAxes.y)
				{
					bodySettings.mAllowedDOFs |= JPH::EAllowedDOFs::RotationY;
				}
				if (!info.freezeRotationAxes.z)
				{
					bodySettings.mAllowedDOFs |= JPH::EAllowedDOFs::RotationZ;
				}
				if (!info.freezeVelocityAxes.x)
				{
					bodySettings.mAllowedDOFs |= JPH::EAllowedDOFs::TranslationX;
				}
				if (!info.freezeVelocityAxes.y)
				{
					bodySettings.mAllowedDOFs |= JPH::EAllowedDOFs::TranslationY;
				}
				if (!info.freezeVelocityAxes.z)
				{
					bodySettings.mAllowedDOFs |= JPH::EAllowedDOFs::TranslationZ;
				}
				
				// --- Create and Add Body ---
				body = joltBodyInterface.CreateBody(bodySettings);
				if (!body)
				{
					std::cerr << "RigidBody ERROR: Failed to create Jolt body for Entity [" << entity.ID << "]." << std::endl;
					return bodyIDAny;
				}
				bodyID = body->GetID(); // Store the new ID
				
				// // Add the body to the simulation (activate it)
				joltBodyInterface.AddBody(bodyID, JPH::EActivation::Activate);
				
				physicsScene.make<scenes::JoltIDComponentPair>("registerRigidBody", bodyID, &component);
				return bodyIDAny;
			}}
		},
		.setDataFunctions = {
			{"attachCollider", [](const std::any& val, auto& component)->void
			{
				auto collider = std::any_cast<components::entities::EntityComponent*>(val);
				auto& colliders = component.template getData<std::vector<components::entities::EntityComponent*>>("Colliders");
				for (const auto* existing : colliders)
				{
					if (existing == collider)
						return;
				}
				colliders.push_back(collider);
				component.template getData<JPH::BodyID>("recreateJoltBody");
			}},
			{"detachCollider", [](const std::any& val, auto& component)->void
			{
				auto collider = std::any_cast<components::entities::EntityComponent*>(val);
				auto& colliders = component.template getData<std::vector<components::entities::EntityComponent*>>("Colliders");
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
				if (changed)
				{
					component.template getData<JPH::BodyID>("recreateJoltBody");
				}
			}
		}}
	};
	return createInfo;
}

// void RigidBody::recreateJoltBody()
// {
// };
// void RigidBody::onAttached()
// {
// 	std::cout << "RigidBody attached." << std::endl;
// }
// void RigidBody::onUpdate() {}
// void RigidBody::onDetached()
// {
// 	physicsScene->unregisterRigidBody(this);
// 	physicsScene = 0;
// }
// void RigidBody::applyForce(glm::vec3 worldForce, glm::vec3 worldPoint)
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	bodyInterface.AddForce(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(worldForce),
// 												 ToJolt<glm::vec3, JPH::Vec3>(worldPoint));
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// void RigidBody::applyLocalForceToCenter(glm::vec3 localForce)
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	auto bodyRotation = bodyInterface.GetRotation(joltBodyID);
// 	auto worldForce = bodyRotation * ToJolt<glm::vec3, JPH::Vec3>(localForce);
// 	bodyInterface.AddForce(joltBodyID, worldForce);
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// void RigidBody::applyForceToCenter(glm::vec3 force)
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	bodyInterface.AddForce(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(force));
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// void RigidBody::applyTorque(glm::vec3 torque)
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	bodyInterface.AddTorque(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(torque));
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// void RigidBody::clearForces()
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	bodyInterface.SetLinearVelocity(joltBodyID, JPH::Vec3::sZero());
// 	bodyInterface.SetAngularVelocity(joltBodyID, JPH::Vec3::sZero());
// }
// const glm::vec3& RigidBody::getPosition() const { return *position; }
// void RigidBody::setPosition(glm::vec3 newPosition)
// {
// 	physicsScene->GetBodyInterface().SetPosition(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(newPosition),
// 																							 JPH::EActivation::Activate);
// }
// const glm::quat& RigidBody::getOrientation() const { return *rotation; }
// void RigidBody::setOrientation(glm::quat newOrientation)
// {
// 	physicsScene->GetBodyInterface().SetRotation(joltBodyID, ToJolt<glm::quat, JPH::Quat>(newOrientation).Normalized(),
// 																							 JPH::EActivation::Activate);
// }
// const glm::vec3 RigidBody::getLinearVelocity() const
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	return ToJolt<JPH::Vec3, glm::vec3>(bodyInterface.GetLinearVelocity(joltBodyID));
// }
// RigidBodyInfo& RigidBody::getInfo() { return info; }
// const RigidBodyInfo& RigidBody::getInfo() const { return info; }
// void RigidBody::setLinearVelocityX(float va)
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	auto currentVel = bodyInterface.GetLinearVelocity(joltBodyID);
// 	bodyInterface.SetLinearVelocity(joltBodyID, JPH::Vec3(va, currentVel.GetY(), currentVel.GetZ()));
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// void RigidBody::setLinearVelocityY(float va)
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	auto currentVel = bodyInterface.GetLinearVelocity(joltBodyID);
// 	bodyInterface.SetLinearVelocity(joltBodyID, JPH::Vec3(currentVel.GetX(), va, currentVel.GetZ()));
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// void RigidBody::setLinearVelocityZ(float va)
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	auto currentVel = bodyInterface.GetLinearVelocity(joltBodyID);
// 	bodyInterface.SetLinearVelocity(joltBodyID, JPH::Vec3(currentVel.GetX(), currentVel.GetY(), va));
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// void RigidBody::setAngularVelocityX(float va)
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	auto currentVel = bodyInterface.GetAngularVelocity(joltBodyID);
// 	bodyInterface.SetAngularVelocity(joltBodyID, JPH::Vec3(va, currentVel.GetY(), currentVel.GetZ()));
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// void RigidBody::setAngularVelocityY(float va)
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	auto currentVel = bodyInterface.GetAngularVelocity(joltBodyID);
// 	bodyInterface.SetAngularVelocity(joltBodyID, JPH::Vec3(currentVel.GetX(), va, currentVel.GetZ()));
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// void RigidBody::setAngularVelocityZ(float va)
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	auto currentVel = bodyInterface.GetAngularVelocity(joltBodyID);
// 	bodyInterface.SetAngularVelocity(joltBodyID, JPH::Vec3(currentVel.GetX(), currentVel.GetY(), va));
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// void RigidBody::setLinearVelocity(glm::vec3 newLinearVelocity)
// {
// 	JPH::BodyInterface& bodyInterface = physicsScene->GetBodyInterface();
// 	bodyInterface.SetLinearVelocity(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(newLinearVelocity));
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// void RigidBody::setAngularVelocity(glm::vec3 newAngularVelocity)
// {
// 	JPH::BodyInterface& bodyInterface = physicsScene->GetBodyInterface();
// 	bodyInterface.SetAngularVelocity(joltBodyID, ToJolt<glm::vec3, JPH::Vec3>(newAngularVelocity));
// 	bodyInterface.ActivateBody(joltBodyID);
// }
// bool RigidBody::isSleeping() { return sleeping; }
// void RigidBody::setSleeping(bool newSleeping)
// {
// 	if (sleeping != newSleeping)
// 		sleeping = newSleeping;
// }
// JPH::BodyID RigidBody::getJoltBodyID() const { return joltBodyID; }
// glm::vec3 RigidBody::getAngularVelocity() const { throw ""; }
// float RigidBody::getMass() const { return info.mass; }
// void RigidBody::setMass(float newMass) { info.mass = newMass; }
// float RigidBody::getInverseMass() const
// {
// 	return (info.bodyType == BodyType::Dynamic && info.mass > 0.0f) ? 1.0f / info.mass : 0.0f;
// }
// const std::vector<Collider*>& RigidBody::getColliders() const { return colliders; }
// bool RigidBody::getUseGravity() const { return info.useGravity; }
// glm::vec3 RigidBody::getFreezeRotationAxes() const { return info.freezeRotationAxes; }
// glm::vec3 RigidBody::getFreezeVelocityAxes() const { return info.freezeVelocityAxes; }
// glm::vec3 RigidBody::getForceAccumulator() const { throw ""; }
// glm::vec3 RigidBody::getTorqueAccumulator() const { throw ""; }
// float RigidBody::getLinearDamping() { return info.linearDamping; }
// float RigidBody::getAngularDamping() { return info.angularDamping; }
// void RigidBody::translate(glm::vec3 deltaPos)
// {
// 	// translate Jolt body
// 	// update *position and *rotation
// }
// bool RigidBody::isStatic() const { return info.bodyType == BodyType::Static; }
// bool RigidBody::isKinematic() const { return info.bodyType == BodyType::Kinematic; }
// bool RigidBody::isDynamic() const { return info.bodyType == BodyType::Dynamic; }
// glm::vec3 RigidBody::getCenterAtTime(float t) const { return *position + getLinearVelocity() * t; }
// bool RigidBody::isTouching(RigidBody& rigidBody, physics::CollisionManifold*& ManifoldPointer)
// {
// 	std::lock_guard lock(mutex);
// 	auto iter = activeRigidBodyManifolds.find(&rigidBody);
// 	if (iter == activeRigidBodyManifolds.end())
// 	{
// 		return false;
// 	}
// 	ManifoldPointer = &iter->second;
// 	return true;
// }
// void RigidBody::addActiveManifold(const physics::CollisionManifold& manifold)
// {
// 	std::lock_guard lock(mutex);
// 	auto rigidBodyPointer = (this == manifold.bodyA) ? manifold.bodyB : manifold.bodyA;
// 	activeRigidBodyManifolds[rigidBodyPointer] = manifold;
// }
// void RigidBody::removeActiveManifold(RigidBody& otherRb)
// {
// 	std::lock_guard lock(mutex);
// 	auto iter = activeRigidBodyManifolds.find(&otherRb);
// 	if (iter == activeRigidBodyManifolds.end())
// 	{
// 		return;
// 	}
// 	activeRigidBodyManifolds.erase(iter);
// }
// void RigidBody::clearActiveManifolds()
// {
// 	std::lock_guard lock(mutex);
// 	activeRigidBodyManifolds.clear();
// }

#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
#include <zg/Registry.hpp>
using namespace zg::components::entities;
RigidBodyInfo::RigidBodyInfo(BodyType bodyType, float mass, float linearDamping, float angularDamping,
	bool useGravity, bool isKinematicInitially, glm::vec<3, bool> freezeRotationAxes,
	glm::vec<3, bool> freezeVelocityAxes, size_t collisionMask):
	bodyType(bodyType),
	mass(mass),
	linearDamping(linearDamping),
	angularDamping(angularDamping),
	useGravity(useGravity),
	isKinematicInitially(isKinematicInitially),
	freezeRotationAxes(freezeRotationAxes),
	freezeVelocityAxes(freezeVelocityAxes),
	collisionMask(collisionMask)
{};
RigidBodyInfo::RigidBodyInfo(const RigidBodyInfo& other):
	bodyType(other.bodyType),
	mass(other.mass),
	linearDamping(other.linearDamping),
	angularDamping(other.angularDamping),
	useGravity(other.useGravity),
	isKinematicInitially(other.isKinematicInitially),
	freezeRotationAxes(other.freezeRotationAxes),
	freezeVelocityAxes(other.freezeVelocityAxes),
	collisionMask(other.collisionMask)
{}
RigidBodyInfo& RigidBodyInfo::operator=(const RigidBodyInfo& other)
{
	bodyType = other.bodyType;
	mass = other.mass;
	linearDamping = other.linearDamping;
	angularDamping = other.angularDamping;
	useGravity = other.useGravity;
	isKinematicInitially = other.isKinematicInitially;
	freezeRotationAxes = other.freezeRotationAxes;
	freezeVelocityAxes = other.freezeVelocityAxes;
	collisionMask = other.collisionMask;
	return *this;
}
zg::components::entities::EntityComponentCreateInfo zg::components::entities::RigidBodyFactory(const RigidBodyInfo& info)
{
	zg::components::entities::EntityComponentCreateInfo createInfo{
		.name = "RigidBody",
		.onAttachedFunction = [&, info](auto& component)
		{
			auto& entity = Registry::getEntity(component.HOST_INDEX_STACK);
			auto& scene = Registry::getScene(component.HOST_INDEX_STACK);
			auto& physicsScene = component.template make<zg::components::scenes::SceneComponent*>("PhysicsScene", nullptr);
			try
			{
				physicsScene = &scene.getComponentByName("PhysicsScene");
			}
			catch(...)
			{
				throw std::runtime_error("RigidBody component must not be added to an entity before the scene the entity is in has "
																	"an attached PhysicsScene component");
			}
			component.template make<RigidBodyInfo>("Info", info);
			component.template make<glm::vec3*>("Position", &entity.position);
			component.template make<glm::quat*>("Rotation", &entity.rotation);
			component.template make<JPH::BodyID>("BodyID");
			component.template make<JPH::BodyInterface*>("BodyInterface", physicsScene->template getData<JPH::BodyInterface*>("BodyInterface"));
			component.template make<JPH::Body*>("Body");
			component.template make<std::vector<components::entities::EntityComponent*>>("Colliders");
			component.template make<std::unordered_map<size_t, physics::CollisionManifold>>("activeRigidBodyManifolds");
			component.template make<std::mutex*>("Mutex", new std::mutex());
			component.template make<std::map<size_t, size_t>>("CollidingMaskCounts");
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
				auto& entity = Registry::getEntity(component.HOST_INDEX_STACK);
				auto& physicsScene = *component.template getData<zg::components::scenes::SceneComponent*>("PhysicsScene");
				auto& info = component.template getData<RigidBodyInfo>("Info");
				auto& position = *component.template getData<glm::vec3*>("Position");
				auto& rotation = *component.template getData<glm::quat*>("Rotation");
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
					auto& colInfo = col->template getData<ColliderInfo>("Info");
					auto baseShape = colInfo.shapeData->createJoltShape();
					if (!baseShape)
					{
						std::cerr << "RigidBody ERROR: Failed to create base Jolt shape for Entity [" << entity.ID << "]."
											<< std::endl;
						return bodyIDAny;
					}
			
					finalShape = baseShape;
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
					const auto& mat = collider0.template getData<PhysicsMaterial>("PhysicsMaterial"); // Use first collider's material
					bodySettings.mFriction = mat.friction;
					bodySettings.mRestitution = mat.restitution;
					// Sensor property - affects the whole body in Jolt unless using specific contact listener logic
					bodySettings.mIsSensor = collider0.template getData<bool>("IsSensor");
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
				
				physicsScene.template setData<scenes::JoltIDComponentPair>("attachRigidBody", scenes::JoltIDComponentPair(bodyID, &component));
				return bodyIDAny;
			}},
			{"CollidingMask", [](auto& component)->std::any&
			{
				auto& collidingMaskCounts = component.template getData<std::map<size_t, size_t>>("CollidingMaskCounts");
				size_t collisionMask = 0;
				for (auto& pair : collidingMaskCounts)
				{
					if (pair.second)
						collisionMask |= pair.first;
				}
				return component.template makeReturnAny<size_t>("m_CollidingMask", collisionMask);
			}},
			{"CollisionMask", [](auto& component)->std::any&
			{
				try
				{
					return component.getDataReturnAny("m_CollisionMask");
				}
				catch (...)
				{}
				auto& info = component.template getData<RigidBodyInfo>("Info");
				return component.template makeReturnAny<size_t>("m_CollisionMask", info.collisionMask);
			}}
		},
		.setDataFunctions = {
			{"attachCollider", [](const std::any& val, auto& component)->std::any
			{
				auto collider = std::any_cast<components::entities::EntityComponent*>(val);
				auto& colliders = component.template getData<std::vector<components::entities::EntityComponent*>>("Colliders");
				for (const auto* existing : colliders)
				{
					if (existing == collider)
						return {};
				}
				colliders.push_back(collider);
				component.template getData<JPH::BodyID>("recreateJoltBody");
				return {};
			}},
			{"detachCollider", [](const std::any& val, auto& component)->std::any
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
				if (changed && colliders.size())
				{
					component.template getData<JPH::BodyID>("recreateJoltBody");
				}
				return {};
			}
		},
			{"isTouching", [](const std::any& val, auto& component)->std::any
				{
					auto& mutexPointer = component.template getData<std::mutex*>("Mutex");
					auto& mutex = *mutexPointer;
					auto& activeRigidBodyManifolds = component.template getData<std::unordered_map<size_t, physics::CollisionManifold>>("activeRigidBodyManifolds");
					auto otherRb = std::any_cast<components::entities::EntityComponent*>(val);
					auto iter = activeRigidBodyManifolds.find(otherRb->ID);
					if (iter == activeRigidBodyManifolds.end())
					{
						return {false};
					}
					return {true};
				}
			},
			{"getTouchingManifold", [](const std::any& val, auto& component)->std::any
				{
					auto& mutexPointer = component.template getData<std::mutex*>("Mutex");
					auto& mutex = *mutexPointer;
					auto& activeRigidBodyManifolds = component.template getData<std::unordered_map<size_t, physics::CollisionManifold>>("activeRigidBodyManifolds");
					auto otherRb = std::any_cast<components::entities::EntityComponent*>(val);
					auto iter = activeRigidBodyManifolds.find(otherRb->ID);
					if (iter == activeRigidBodyManifolds.end())
					{
						throw std::runtime_error("RigidBody is not touching other RigidBody");
					}
					return {iter->second};
				}
			},
			{"addActiveManifold", [](const std::any& val, auto& component)->std::any {
				auto& mutexPointer = component.template getData<std::mutex*>("Mutex");
				auto& mutex = *mutexPointer;
				auto& activeRigidBodyManifolds = component.template getData<std::unordered_map<size_t, physics::CollisionManifold>>("activeRigidBodyManifolds");
				auto manifold = std::any_cast<physics::CollisionManifold>(val);
				std::lock_guard lock(mutex);
				auto rigidBodyComponentPointer = (&component == manifold.ecA) ? manifold.ecB : manifold.ecA;
				activeRigidBodyManifolds[rigidBodyComponentPointer->ID] = manifold;
				auto& collidingMaskCounts = component.template getData<std::map<size_t, size_t>>("CollidingMaskCounts");
				auto& rbCollisionMask = rigidBodyComponentPointer->template getData<size_t>("CollisionMask");
				collidingMaskCounts[rbCollisionMask]++;
				return {};
			}},
			{"removeActiveManifold", [](const std::any& val, auto& component)->std::any {
				auto& mutexPointer = component.template getData<std::mutex*>("Mutex");
				auto& mutex = *mutexPointer;
				auto& activeRigidBodyManifolds = component.template getData<std::unordered_map<size_t, physics::CollisionManifold>>("activeRigidBodyManifolds");
				std::lock_guard lock(mutex);
				auto otherRb = std::any_cast<components::entities::EntityComponent*>(val);
				auto iter = activeRigidBodyManifolds.find(otherRb->ID);
				if (iter == activeRigidBodyManifolds.end())
				{
					return {};
				}
				auto& collidingMaskCounts = component.template getData<std::map<size_t, size_t>>("CollidingMaskCounts");
				auto& rbCollisionMask = otherRb->template getData<size_t>("CollisionMask");
				collidingMaskCounts[rbCollisionMask]--;
				activeRigidBodyManifolds.erase(iter);
				return {};
			}},
			{"applyForce", [](const auto& val, auto& component)->std::any {
				auto& worldForcePointPair = std::any_cast<const std::pair<glm::vec3, glm::vec3>&>(val);
				auto& worldForce = worldForcePointPair.first;
				auto& worldPoint = worldForcePointPair.second;
				auto& joltBodyInterfacePointer = component.template getData<JPH::BodyInterface*>("BodyInterface");
				auto& bodyInterface = *joltBodyInterfacePointer;
				auto& bodyIDAny = component.getDataReturnAny("BodyID");
				auto& bodyID = std::any_cast<JPH::BodyID&>(bodyIDAny);
				bodyInterface.AddForce(bodyID, ToJolt<glm::vec3, JPH::Vec3>(worldForce),
																ToJolt<glm::vec3, JPH::Vec3>(worldPoint));
				bodyInterface.ActivateBody(bodyID);
				return {};
			}},
			{"applyLocalForceToCenter", [](const auto& val, auto& component)->std::any {
				auto& localForce = std::any_cast<const glm::vec3&>(val);
				auto& joltBodyInterfacePointer = component.template getData<JPH::BodyInterface*>("BodyInterface");
				auto& bodyInterface = *joltBodyInterfacePointer;
				auto& bodyIDAny = component.getDataReturnAny("BodyID");
				auto& bodyID = std::any_cast<JPH::BodyID&>(bodyIDAny);
				auto bodyRotation = bodyInterface.GetRotation(bodyID);
				auto worldForce = bodyRotation * ToJolt<glm::vec3, JPH::Vec3>(localForce);
				bodyInterface.AddForce(bodyID, worldForce);
				bodyInterface.ActivateBody(bodyID);
				return {};
			}},
			{"applyForceToCenter", [](const auto& val, auto& component)->std::any {
				auto& force = std::any_cast<const glm::vec3&>(val);
				auto& joltBodyInterfacePointer = component.template getData<JPH::BodyInterface*>("BodyInterface");
				auto& bodyInterface = *joltBodyInterfacePointer;
				auto& bodyIDAny = component.getDataReturnAny("BodyID");
				auto& bodyID = std::any_cast<JPH::BodyID&>(bodyIDAny);
				bodyInterface.AddForce(bodyID, ToJolt<glm::vec3, JPH::Vec3>(force));
				bodyInterface.ActivateBody(bodyID);
				return {};
			}},
			{"applyTorque", [](const auto& val, auto& component)->std::any {
				auto& torque = std::any_cast<const glm::vec3&>(val);
				auto& joltBodyInterfacePointer = component.template getData<JPH::BodyInterface*>("BodyInterface");
				auto& bodyInterface = *joltBodyInterfacePointer;
				auto& bodyIDAny = component.getDataReturnAny("BodyID");
				auto& bodyID = std::any_cast<JPH::BodyID&>(bodyIDAny);
				bodyInterface.AddTorque(bodyID, ToJolt<glm::vec3, JPH::Vec3>(torque));
				bodyInterface.ActivateBody(bodyID);
				return {};
			}},
			{"setPosition", [](const auto& val, auto& component)->std::any {
				auto& newPosition = std::any_cast<const glm::vec3&>(val);
				auto& joltBodyInterfacePointer = component.template getData<JPH::BodyInterface*>("BodyInterface");
				auto& bodyInterface = *joltBodyInterfacePointer;
				auto& bodyIDAny = component.getDataReturnAny("BodyID");
				auto& bodyID = std::any_cast<JPH::BodyID&>(bodyIDAny);
				bodyInterface.SetPosition(bodyID, ToJolt<glm::vec3, JPH::Vec3>(newPosition), JPH::EActivation::Activate);
				return {};
				
			}},
			{"setOrientation", [](const auto& val, auto& component)->std::any {
				auto& newOrientation = std::any_cast<const glm::quat&>(val);
				auto& joltBodyInterfacePointer = component.template getData<JPH::BodyInterface*>("BodyInterface");
				auto& bodyInterface = *joltBodyInterfacePointer;
				auto& bodyIDAny = component.getDataReturnAny("BodyID");
				auto& bodyID = std::any_cast<JPH::BodyID&>(bodyIDAny);
				bodyInterface.SetRotation(bodyID, ToJolt<glm::quat, JPH::Quat>(newOrientation).Normalized(), JPH::EActivation::Activate);
				return {};
			}}
		}
	};
	return createInfo;
}

// void RigidBody::clearForces()
// {
// 	auto& bodyInterface = physicsScene->GetBodyInterface();
// 	bodyInterface.SetLinearVelocity(joltBodyID, JPH::Vec3::sZero());
// 	bodyInterface.SetAngularVelocity(joltBodyID, JPH::Vec3::sZero());
// }
// const glm::vec3& RigidBody::getPosition() const { return *position; }
// const glm::quat& RigidBody::getOrientation() const { return *rotation; }
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

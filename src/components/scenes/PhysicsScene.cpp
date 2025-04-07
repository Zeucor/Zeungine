#include <zg/Scene.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/IGravity.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
using namespace zg::components::scenes;
PhysicsScene::PhysicsScene(Scene& scene) :
		ISceneComponent("PhysicsScene"), scene(scene), deltaTime(scene.window.deltaTime)
{
	std::cout << "PhysicsScene created." << std::endl;
}
void PhysicsScene::onAttached()
{
	gravity = dynamic_cast<IGravity*>(scene.getComponentByName("IGravity").get());
	if (!gravity)
	{
		throw std::runtime_error("PhysicsScene requires an IGravity component to be added before adding to a Scene");
	}
	timeAccumulator = 0.0;
	rigidBodies.clear();
	std::cout << "PhysicsScene attached." << std::endl;
}
void PhysicsScene::onUpdate()
{
	timeAccumulator += deltaTime;
	int subSteps = 0;
	while (timeAccumulator >= fixedTimeStep && subSteps < maxSubSteps)
	{
		stepSimulation(fixedTimeStep);
		timeAccumulator -= fixedTimeStep;
		subSteps++;
	}
	if (subSteps >= maxSubSteps && timeAccumulator >= fixedTimeStep)
	{
		std::cerr << "Warning: Physics simulation lagging behind real-time." << std::endl;
		timeAccumulator = std::fmod(timeAccumulator, fixedTimeStep);
	}
	synchronizeTransforms();
}
void PhysicsScene::onDetached()
{
	rigidBodies.clear();
	gravity = nullptr; // Clear pointer, don't delete if owned by scene
	std::cout << "PhysicsScene detached." << std::endl;
}
void PhysicsScene::registerRigidBody(entities::RigidBody* rigidBody)
{
	if (std::find(rigidBodies.begin(), rigidBodies.end(), rigidBody) == rigidBodies.end())
	{
		rigidBodies.push_back(rigidBody);
		std::cout << "Registered RigidBody." << std::endl;
	}
}
void PhysicsScene::unregisterRigidBody(entities::RigidBody* rigidBody)
{
	if (rigidBody)
	{
		auto it = std::remove(rigidBodies.begin(), rigidBodies.end(), rigidBody);
		if (it != rigidBodies.end())
		{
			rigidBodies.erase(it, rigidBodies.end());
			std::cout << "Unregistered RigidBody." << std::endl;
		}
	}
}
void PhysicsScene::stepSimulation(long double dt) {}
void PhysicsScene::integrate(long double dt)
{

	for (entities::RigidBody* body : rigidBodies)
	{
		if (!body->isDynamic())
			continue; // Static and Kinematic bodies aren't integrated by forces

		// --- Linear Motion ---
		// Acceleration = Force / Mass (or Force * InverseMass)
		glm::vec3 linearAcceleration = body->forceAccumulator * body->inverseMass;

		// Update linear velocity: v_new = v_old + a * dt
		body->linearVelocity += linearAcceleration * (float)dt;

		// Apply linear damping: v_new = v_old * damping_factor
		// Using pow is more stable for varying dt, but (1 - damp * dt) is simpler if dt is fixed.
		// Let's use pow for robustness with the fixed timestep system.
		body->linearVelocity *= std::pow(1.0f - body->info.linearDamping, dt);

		// Apply position freeze constraints (World Axes)
		if (body->info.freezeVelocityAxes.x)
			body->linearVelocity.x = 0;
		if (body->info.freezeVelocityAxes.y)
			body->linearVelocity.y = 0;
		if (body->info.freezeVelocityAxes.z)
			body->linearVelocity.z = 0;

		// Calculate new position: p_new = p_old + v_new * dt
		// PROBLEM: We cannot update the transform directly as we only have const glm::mat4*
		// We calculate where it *should* be, but can't set it back.
		if (body->transform)
		{
			glm::vec3 currentPosition = glm::vec3((*body->transform)[3]); // Extract current position
			glm::vec3 newPosition = currentPosition + body->linearVelocity * (float)dt;
			// Normally: body->getTransformComponent()->setPosition(newPosition);
			// std::cout << "New Pos Calc: " << newPosition.x << "," << newPosition.y << "," << newPosition.z << std::endl; //
			// Debug
		}


		// --- Angular Motion ---
		// TODO: Needs Inverse Inertia Tensor for correctness
		// Angular Acceleration (alpha) = Torque / Inertia (or Torque * InverseInertiaTensor)
		// glm::vec3 angularAcceleration = body->inverseInertiaTensor * body->torqueAccumulator; // Placeholder

		// For now, let's assume zero angular acceleration if tensor isn't implemented
		glm::vec3 angularAcceleration = {0.0f, 0.0f, 0.0f}; // TEMP

		// Update angular velocity: w_new = w_old + alpha * dt
		body->angularVelocity += angularAcceleration * (float)dt;

		// Apply angular damping
		body->angularVelocity *= std::pow(1.0f - body->info.angularDamping, (float)dt);

		// Apply rotation freeze constraints (World Axes)
		if (body->info.freezeRotationAxes.x)
			body->angularVelocity.x = 0;
		if (body->info.freezeRotationAxes.y)
			body->angularVelocity.y = 0;
		if (body->info.freezeRotationAxes.z)
			body->angularVelocity.z = 0;

		// Calculate new rotation: q_new = deltaRotation * q_old
		// deltaRotation is angle-axis rotation based on w_new * dt
		// PROBLEM: Cannot update the transform directly.
		if (body->transform && glm::length(body->angularVelocity) > 1e-6f) // Avoid division by zero if w is zero
		{
			glm::quat currentRotation = glm::quat_cast(*body->transform); // Extract current rotation
			float angle = glm::length(body->angularVelocity) * (float)dt;
			glm::vec3 axis = glm::normalize(body->angularVelocity);
			glm::quat rotationDelta = glm::angleAxis(angle, axis);
			glm::quat newRotation = rotationDelta * currentRotation;
			newRotation = glm::normalize(newRotation); // Keep quaternion normalized
			// Normally: body->getTransformComponent()->setRotation(newRotation);
			// std::cout << "New Rot Calc: " << newRotation.w << "," << newRotation.x << "," << newRotation.y << "," <<
			// newRotation.z << std::endl; // Debug
		}
	}
}
void PhysicsScene::detectCollisions() {}
void PhysicsScene::resolveCollisions() {}
void PhysicsScene::synchronizeTransforms() {}

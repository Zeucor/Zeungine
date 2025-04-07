#include <zg/Scene.hpp>
#include <zg/components/entities/Collider.hpp>
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
void PhysicsScene::stepSimulation(long double dt)
{
	// 1. Apply global forces (like gravity)
	if (gravity)
	{
		gravity->applyGravity(*this);
	}

	// 2. Integrate motion (update velocities and predict new positions/rotations)
	integrate(dt);

	// 3. Collision Detection (find pairs that are colliding)
	detectCollisions();

	// 4. Collision Resolution (apply impulses and correct penetration)
	// Consider multiple resolution iterations for stability (e.g., sequential impulses)
	const int resolutionIterations = 5; // Example: Number of resolution passes
	for (int i = 0; i < resolutionIterations; ++i)
	{
		resolveCollisions(dt); // Pass dt for potential time-dependent effects (like friction)
	}


	// 5. Clear forces for the next step
	for (auto body : rigidBodies)
	{
		if (body)
			body->clearForces();
	}
}
void PhysicsScene::integrate(long double dt)
{
	float fdt = static_cast<float>(dt); // Use float for glm calculations

	for (auto body : rigidBodies)
	{
		if (!body || !body->isDynamic() || !body->transform)
			continue; // Skip non-dynamic, null, or bodies without transform

		// --- Linear Motion ---
		glm::vec3 linearAcceleration = body->forceAccumulator * body->inverseMass;
		body->linearVelocity += linearAcceleration * fdt;
		// Apply damping: dv = -damping * v * dt => v_new = v * (1 - damping * dt)
		// More stable damping: v_new = v / (1 + damping * dt) or pow(1-damping, dt) is also common
		body->linearVelocity *= std::pow(1.0f - body->info.linearDamping, fdt);

		// Apply velocity constraints
		if (body->info.freezeVelocityAxes.x)
			body->linearVelocity.x = 0;
		if (body->info.freezeVelocityAxes.y)
			body->linearVelocity.y = 0;
		if (body->info.freezeVelocityAxes.z)
			body->linearVelocity.z = 0;

		// Update position (using the RigidBody's translate method)
		body->translate(body->linearVelocity * fdt);


		// --- Angular Motion ---
		// TODO: Needs Inverse Inertia Tensor implementation
		// glm::vec3 angularAcceleration = body->inverseInertiaTensor * body->torqueAccumulator; // Correct calculation
		glm::vec3 angularAcceleration = {0.0f, 0.0f, 0.0f}; // Placeholder
		body->angularVelocity += angularAcceleration * fdt;
		body->angularVelocity *= std::pow(1.0f - body->info.angularDamping, fdt);

		// Apply rotation constraints
		if (body->info.freezeRotationAxes.x)
			body->angularVelocity.x = 0;
		if (body->info.freezeRotationAxes.y)
			body->angularVelocity.y = 0;
		if (body->info.freezeRotationAxes.z)
			body->angularVelocity.z = 0;

		// Update rotation (using the RigidBody's setRotation method)
		if (glm::length2(body->angularVelocity) > 1e-9f) // Use length2 for efficiency, adjust threshold
		{
			float angle = glm::length(body->angularVelocity) * fdt;
			glm::vec3 axis = glm::normalize(body->angularVelocity);
			glm::quat rotationDelta = glm::angleAxis(angle, axis);
			// Apply delta: newRotation = delta * oldRotation
			body->setRotation(rotationDelta * body->getRotation());
		}
	}
}
void PhysicsScene::detectCollisions()
{
	potentialPairs.clear();
	collisionContacts.clear(); // Clear contacts from previous step

	// --- 1. Broadphase (Example: Simple N^2 AABB Check) ---
	// Optimization: Use spatial partitioning (Grid, Octree, BVH) for large scenes
	for (size_t i = 0; i < rigidBodies.size(); ++i)
	{
		auto bodyA = rigidBodies[i];
		if (!bodyA || bodyA->colliders.empty() || !bodyA->transform)
			continue;

		for (size_t j = i + 1; j < rigidBodies.size(); ++j)
		{
			auto bodyB = rigidBodies[j];
			if (!bodyB || bodyB->colliders.empty() || !bodyB->transform)
				continue;

			// Basic filtering: Don't check static vs static
			if (bodyA->isStatic() && bodyB->isStatic())
				continue;
			// Add other filters if needed (e.g., layers, kinematic vs kinematic)

			// Check AABB overlap for all collider pairs between bodyA and bodyB
			for (auto colliderA : bodyA->colliders)
			{
				if (!colliderA)
					continue;
				// colliderA->updateWorldAABB(); // Ensure AABB is current (call if needed)

				for (auto colliderB : bodyB->colliders)
				{
					if (!colliderB)
						continue;
					// colliderB->updateWorldAABB(); // Ensure AABB is current

					// Broadphase check
					if (colliderA->getWorldAABB().overlaps(colliderB->getWorldAABB()))
					{
						potentialPairs.push_back({colliderA, colliderB});
					}
				}
			}
		}
	}

	// --- 2. Narrowphase (Check potential pairs more precisely) ---
	for (const auto& pair : potentialPairs)
	{
		auto colliderA = pair.first;
		auto colliderB = pair.second;

		// Example: Box-Box check using SAT
		if (colliderA->getShapeType() == entities::ShapeType::Box && colliderB->getShapeType() == entities::ShapeType::Box)
		{
			physics::CollisionManifold manifold(colliderA, colliderB);
			if (performSATBoxBox(colliderA, colliderB, manifold))
			{
				// Collision detected by SAT, add the filled manifold
				collisionContacts.push_back(manifold);
			}
		}
		// TODO: Add checks for other shape combinations (Sphere-Sphere, Sphere-Box, etc.)
		// else if (colliderA->getShapeType() == ShapeType::Sphere && ...) { ... }
	}
	// Debugging: Print number of contacts found
	// if (!collisionContacts.empty()) {
	//     std::cout << "Narrowphase detected contacts: " << collisionContacts.size() << std::endl;
	// }
}
bool PhysicsScene::performSATBoxBox(entities::Collider* boxA, entities::Collider* boxB,
																		physics::CollisionManifold& manifold)
{
	// --- Input Validation ---
	if (!boxA || boxA->getShapeType() != entities::ShapeType::Box || !boxB ||
			boxB->getShapeType() != entities::ShapeType::Box)
	{
		return false; // Invalid input
	}
	auto bodyA = boxA->getOwnerRigidBody();
	auto bodyB = boxB->getOwnerRigidBody();
	if (!bodyA || !bodyB)
	{
		// Need rigid bodies to get transforms and positions
		std::cerr << "Warning: SATBoxBox requires colliders to have RigidBody owners." << std::endl;
		return false;
	}


	// --- SAT Initialization ---
	float minPenetration = std::numeric_limits<float>::infinity();
	glm::vec3 collisionNormal(0.0f);
	bool separatingAxisFound = false; // Flag to track if we found *any* separating axis

	// --- Get Axes to Test ---
	// Get world-aligned axes for both boxes (3 each)
	std::vector<glm::vec3> axesA = getBoxWorldAxes(boxA);
	std::vector<glm::vec3> axesB = getBoxWorldAxes(boxB);

	// Combine axes: 3 from A, 3 from B, up to 9 from cross products
	std::vector<glm::vec3> testAxes;
	testAxes.reserve(15); // 3 + 3 + 9 = 15 potential axes
	testAxes.insert(testAxes.end(), axesA.begin(), axesA.end());
	testAxes.insert(testAxes.end(), axesB.begin(), axesB.end());

	// Generate cross product axes (edge-edge directions)
	for (const auto& axisA : axesA)
	{
		for (const auto& axisB : axesB)
		{
			glm::vec3 crossAxis = glm::cross(axisA, axisB);
			// Check for non-zero length (handles parallel axes) using squared length for efficiency
			if (glm::length2(crossAxis) > 1e-8f)
			{ // Use a small epsilon
				testAxes.push_back(glm::normalize(crossAxis));
			}
		}
	}

	// --- Test Projections on Each Axis ---
	for (const auto& axis : testAxes)
	{
		// Check for degenerate axes (should be rare after normalization, but safe)
		if (glm::length2(axis) < 1e-8f)
			continue;

		float minA, maxA, minB, maxB;
		projectBoxOntoAxis(boxA, axis, minA, maxA);
		projectBoxOntoAxis(boxB, axis, minB, maxB);

		// Calculate overlap on this axis
		// overlap = min(maxA, maxB) - max(minA, minB)
		float currentOverlap = glm::min(maxA, maxB) - glm::max(minA, minB);

		// --- Check for Separation ---
		if (currentOverlap < 0.0f)
		{
			// Found a separating axis! No collision possible.
			separatingAxisFound = true;
			break; // Exit the loop early
		}

		// --- Update Minimum Penetration ---
		// **CRITICAL CHANGE:** Update penetration based on *any* axis that shows overlap.
		// The axis with the *smallest* overlap corresponds to the minimum penetration depth.
		if (currentOverlap < minPenetration)
		{
			minPenetration = currentOverlap;
			collisionNormal = axis; // Store the axis associated with this minimum overlap
		}
	}

	// --- Final Result ---
	if (separatingAxisFound)
	{
		return false; // No collision because a separating axis was found
	}

	// If no separating axis was found after checking all axes, the boxes are colliding.
	// We should have a valid collisionNormal and minPenetration by now.

	// --- Finalize Manifold ---
	// Ensure the normal points from B to A (or consistently, e.g., A to B)
	// Get approximate centers using RigidBody position + Collider offset
	glm::vec3 centerA = bodyA->getPosition() + (bodyA->getRotation() * boxA->getOffset()); // Apply rotation to offset
	glm::vec3 centerB = bodyB->getPosition() + (bodyB->getRotation() * boxB->getOffset()); // Apply rotation to offset
	glm::vec3 directionBA = centerA - centerB; // Vector from B's center to A's center

	// Flip the normal if it's pointing in the "wrong" direction relative to the centers
	if (glm::dot(collisionNormal, directionBA) < 0.0f)
	{
		collisionNormal = -collisionNormal; // Ensure normal points roughly from B towards A
	}

	manifold.colliding = true;
	manifold.normal = glm::normalize(collisionNormal); // Ensure normal is unit length
	manifold.penetrationDepth = minPenetration;
	// manifold.colliderA = boxA; // Already set in detectCollisions
	// manifold.colliderB = boxB; // Already set in detectCollisions

	// TODO: Calculate Contact Points (Advanced)
	// This often involves finding the features (vertices, edges, faces) that correspond
	// to the minimum penetration axis and then clipping them against each other.
	// For box-box, common methods include Sutherland-Hodgman clipping or finding
	// the intersection of the relevant features.
	// As a placeholder, you could use the point on boxA closest to boxB along the normal,
	// or the midpoint of the overlap interval projected back onto the boxes.
	// Example placeholder: Contact point approx center of overlap
	// glm::vec3 contactPoint = centerB + collisionNormal * (glm::length(centerA - centerB) - minPenetration * 0.5f);
	// manifold.contactPoints.push_back(contactPoint);

	return true; // Collision detected
}
void PhysicsScene::resolveCollisions(long double dt)
{
	float fdt = static_cast<float>(dt); // Use float for glm
	if (fdt <= 1e-9f)
		return; // Avoid division by zero if dt is extremely small

	// --- Constants for Resolution ---
	const float slop = 0.01f; // Allowed penetration before correction kicks in strongly
	const float baumgarteBeta = 0.2f; // Baumgarte stabilization factor (0.1 - 0.3 typical)
	const float velocityCorrectionThreshold = 0.01f; // Min relative velocity to apply full restitution

	for (physics::CollisionManifold& manifold : collisionContacts)
	{
		if (!manifold.colliding)
			continue;

		auto colliderA = manifold.colliderA;
		auto colliderB = manifold.colliderB;
		auto bodyA = colliderA ? colliderA->getOwnerRigidBody() : nullptr;
		auto bodyB = colliderB ? colliderB->getOwnerRigidBody() : nullptr;

		if (!bodyA || !bodyB || !colliderA || !colliderB)
			continue;
		if (bodyA->isStatic() && bodyB->isStatic())
			continue;
		if (colliderA->getIsSensor() || colliderB->getIsSensor())
			continue;

		// --- 1. Calculate Relative Velocity ---
		// Simplified: Linear velocity only
		// TODO: Incorporate angular velocity and contact points:
		// glm::vec3 rA = contactPoint - bodyA->getPosition();
		// glm::vec3 rB = contactPoint - bodyB->getPosition();
		// glm::vec3 vA = bodyA->linearVelocity + glm::cross(bodyA->angularVelocity, rA);
		// glm::vec3 vB = bodyB->linearVelocity + glm::cross(bodyB->angularVelocity, rB);
		// glm::vec3 relativeVelocity = vB - vA;
		glm::vec3 relativeVelocity = bodyB->linearVelocity - bodyA->linearVelocity;
		float velocityAlongNormal = glm::dot(relativeVelocity, manifold.normal);

		// --- 2. Calculate Effective Mass & Restitution ---
		float restitution =
			std::min(colliderA->getPhysicsMaterial().restitution, colliderB->getPhysicsMaterial().restitution);

		float invMassSum = bodyA->inverseMass + bodyB->inverseMass;
		// TODO: Include angular component in effective mass when using contact points/angular velocity
		// float invEffectiveMass = invMassSum + dot(cross(invIA * cross(rA, n), rA), n) + dot(cross(invIB * cross(rB, n),
		// rB), n);
		if (invMassSum <= 1e-9f)
			continue; // Both static/immovable
		float effectiveMass = 1.0f / invMassSum; // Simplified linear effective mass

		// --- 3. Calculate Impulse Magnitude (j) with Baumgarte Stabilization ---

		// Calculate Baumgarte bias velocity (to correct positional error)
		// Bias = (beta / dt) * max(0, penetration - slop)
		float penetrationError = std::max(0.0f, manifold.penetrationDepth - slop);
		float baumgarteBias = (baumgarteBeta / fdt) * penetrationError;

		// Calculate restitution velocity (bounce velocity)
		// Only apply restitution if closing velocity is significant
		float restitutionVelocity = 0.0f;
		// Note: velocityAlongNormal is negative for closing velocity
		if (velocityAlongNormal < -velocityCorrectionThreshold)
		{
			restitutionVelocity = -restitution * velocityAlongNormal; // This will be positive
		}

		// Desired velocity change = -(relative_velocity + restitution_velocity + baumgarte_bias)
		// We want the final relative velocity along the normal to be >= restitutionVelocity
		// The impulse needs to cancel the current velocity, add restitution, and add the Baumgarte bias.
		float deltaVelocity = -velocityAlongNormal + restitutionVelocity + baumgarteBias;

		// Impulse magnitude j = deltaVelocity * effectiveMass
		float j = deltaVelocity * effectiveMass;

		// Clamp impulse: Impulse should always be repulsive (non-negative)
		// In sequential impulse solvers, we often clamp the *accumulated* impulse,
		// but here we clamp the impulse for this single contact resolution step.
		j = std::max(0.0f, j);


		// --- Debug Logging (Uncomment to use) ---
		// if (penetrationError > 0 || std::abs(velocityAlongNormal) > 1e-4f) { // Log only significant events
		//     std::cout << "Collision: Pen=" << manifold.penetrationDepth
		//               << ", VRelN=" << velocityAlongNormal
		//               << ", Bias=" << baumgarteBias
		//               << ", RestV=" << restitutionVelocity
		//               << ", j=" << j << std::endl;
		// }


		// --- 4. Apply Impulse ---
		glm::vec3 impulse = manifold.normal * j;

		if (bodyA->isDynamic())
		{
			bodyA->linearVelocity -= impulse * bodyA->inverseMass;
			// TODO: Apply angular impulse: bodyA->angularVelocity -= bodyA->inverseInertiaTensor * glm::cross(rA, impulse);
		}
		if (bodyB->isDynamic())
		{
			bodyB->linearVelocity += impulse * bodyB->inverseMass;
			// TODO: Apply angular impulse: bodyB->angularVelocity += bodyB->inverseInertiaTensor * glm::cross(rB, impulse);
		}

		// --- 5. Positional Correction (Optional with Baumgarte) ---
		// Baumgarte aims to resolve penetration via impulse bias.
		// Applying direct positional correction *in addition* might lead to overcorrection or instability.
		// It's often recommended to use *either* Baumgarte *or* direct positional correction, or use them carefully
		// together. Let's comment out the direct positional correction for now to rely on Baumgarte.
		/*
		if (manifold.penetrationDepth > slop) {
				 float penetrationToCorrect = manifold.penetrationDepth - slop;
				 // Correction amount should not depend on invMassSum being > 0, as that's checked earlier.
				 const float positionalCorrectionPercent = 0.3f; // Keep this factor if using direct correction
				 glm::vec3 correctionVector = manifold.normal * (penetrationToCorrect * positionalCorrectionPercent /
		invMassSum); // Distribute correction

				 if (bodyA->isDynamic() && bodyA->transform) {
						 bodyA->translate(-correctionVector * bodyA->inverseMass);
				 }
				 if (bodyB->isDynamic() && bodyB->transform) {
						 bodyB->translate(correctionVector * bodyB->inverseMass);
				 }
		}
		*/
	}
}
void PhysicsScene::synchronizeTransforms() {}
void PhysicsScene::projectBoxOntoAxis(entities::Collider* boxCollider, const glm::vec3& axis, float& minProj,
																			float& maxProj)
{
	// Ensure valid box and associated data/transform
	if (!boxCollider || boxCollider->getShapeType() != entities::ShapeType::Box)
	{
		minProj = maxProj = 0.0f;
		return;
	}
	const auto* boxData = static_cast<const entities::BoxShapeData*>(boxCollider->getColliderInfo().shapeData.get());
	const glm::mat4* worldTransformPtr = boxCollider->getTransform(); // Get from collider's owner RB
	if (!boxData || !worldTransformPtr)
	{
		minProj = maxProj = 0.0f;
		return;
	}

	// Combine world transform with collider's local offset and rotation
	glm::mat4 localOffsetTransform =
		glm::translate(glm::mat4(1.0f), boxCollider->getOffset()) * glm::mat4_cast(boxCollider->getRotationOffset());
	glm::mat4 finalTransform = (*worldTransformPtr) * localOffsetTransform;

	// Box half extents
	const glm::vec3 h = boxData->halfExtents;

	// Calculate the 8 vertices of the box in its local space (relative to collider center)
	glm::vec3 localVertices[8] = {{-h.x, -h.y, -h.z}, {h.x, -h.y, -h.z}, {h.x, h.y, -h.z}, {-h.x, h.y, -h.z},
																{-h.x, -h.y, h.z},	{h.x, -h.y, h.z},	 {h.x, h.y, h.z},	 {-h.x, h.y, h.z}};

	// Initialize min/max projection values
	minProj = std::numeric_limits<float>::max();
	maxProj = std::numeric_limits<float>::lowest(); // Use lowest() for negative infinity equivalent

	// Project each vertex onto the axis
	for (int i = 0; i < 8; ++i)
	{
		// Transform vertex to world space using the combined transform
		glm::vec4 worldVertex4 = finalTransform * glm::vec4(localVertices[i], 1.0f);
		glm::vec3 worldVertex = glm::vec3(worldVertex4); // Convert from vec4

		// Project onto the axis (dot product)
		float projection = glm::dot(worldVertex, axis);

		// Update min and max
		minProj = std::min(minProj, projection);
		maxProj = std::max(maxProj, projection);
	}
}
std::vector<glm::vec3> PhysicsScene::getBoxWorldAxes(entities::Collider* boxCollider)
{
	std::vector<glm::vec3> worldAxes;
	worldAxes.reserve(3);

	// Default axes if something is wrong
	auto addDefaultAxes = [&]()
	{
		worldAxes.push_back({1.0f, 0.0f, 0.0f});
		worldAxes.push_back({0.0f, 1.0f, 0.0f});
		worldAxes.push_back({0.0f, 0.0f, 1.0f});
	};

	if (!boxCollider)
	{
		addDefaultAxes();
		return worldAxes;
	}

	const glm::mat4* worldTransformPtr = boxCollider->getTransform(); // From owner RB
	if (!worldTransformPtr)
	{
		addDefaultAxes();
		return worldAxes;
	}

	// Combine world rotation with collider's local rotation offset
	glm::quat worldRot = glm::quat_cast(*worldTransformPtr);
	glm::quat localRot = boxCollider->getRotationOffset();
	glm::quat finalRot = worldRot * localRot; // Combine rotations

	// Convert final rotation quaternion to a 3x3 matrix (or use columns of 4x4)
	glm::mat3 rotationMatrix = glm::mat3_cast(finalRot);

	// Extract axes (columns of the rotation matrix) and normalize
	// Normalization is crucial if the original transform has non-uniform scaling
	worldAxes.push_back(glm::normalize(rotationMatrix[0])); // X-axis
	worldAxes.push_back(glm::normalize(rotationMatrix[1])); // Y-axis
	worldAxes.push_back(glm::normalize(rotationMatrix[2])); // Z-axis

	return worldAxes;
}

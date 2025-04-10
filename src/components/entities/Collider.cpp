#include <zg/Entity.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/physics/AABB.hpp>
using namespace zg::components::entities;
using zg::physics::AABB;
BoxShapeData::BoxShapeData(glm::vec3 halfExtents) : halfExtents(halfExtents) {}
MeshShapeData::MeshShapeData(Entity& entity) : entity(entity) {}
Collider::Collider(const ColliderInfo& info) :
		IEntityComponent("Collider"), info(info), transform(&info.entity.getModelMatrix()),
		deltaTime(info.entity.window.deltaTime)
{
}
void Collider::onAttached()
{
	ownerRigidBody = dynamic_cast<RigidBody*>(info.entity.getComponentByName("RigidBody").get());
	if (!ownerRigidBody)
	{
		throw std::runtime_error("Collider Entity[" + std::to_string(info.entity.ID) + ", " + info.entity.name +
														 "] has no RigidBody component. You must add one before adding a Collider");
	}
	ownerRigidBody->addCollider(this);
	updateWorldAABB();
	std::cout << "Collider attached." << std::endl;
}
void Collider::onUpdate()
{
	if (ownerRigidBody && !ownerRigidBody->isStatic())
	{
		updateWorldAABB();
	}
}
void Collider::onDetached()
{
	if (!ownerRigidBody)
	{
		throw std::runtime_error("Collider Entity[" + std::to_string(info.entity.ID) + ", " + info.entity.name +
														 "] has no RigidBody component during detachment. This should not happen");
	}
	else
	{
		// Unregister from the owning RigidBody
		ownerRigidBody->removeCollider(this);
	}
	ownerRigidBody = nullptr;
	transform = nullptr;
	std::cout << "Collider detached." << std::endl;
}
void Collider::updateWorldAABB()
{
	if (!ownerRigidBody || !ownerRigidBody->transform)
	{
		worldAABB._min = worldAABB._max = glm::vec3(0.0f);
		return;
	}

	// Combine world transform with collider's local offset and rotation
	glm::mat4 localOffsetTransform = glm::translate(glm::mat4(1.0f), info.offset) * glm::mat4_cast(info.rotationOffset);
	glm::mat4 finalTransform = (*ownerRigidBody->transform) * localOffsetTransform;
	auto shapeType = info.shapeData ? info.shapeData->getType() : ShapeType::_Count;
	auto shape = info.shapeData ? info.shapeData.get() : (ShapeData*)0;
	auto& body = *ownerRigidBody;

	if (shapeType == ShapeType::Box)
	{
		const auto* boxData = static_cast<const BoxShapeData*>(shape);
		if (!boxData)
		{
			worldAABB._min = worldAABB._max = body.getPosition();
			return;
		}
		const glm::vec3 h = boxData->halfExtents;
		glm::vec3 localVertices[8] = {/* ... box vertices ... */
																	{-h.x, -h.y, -h.z}, {h.x, -h.y, -h.z}, {h.x, h.y, -h.z}, {-h.x, h.y, -h.z},
																	{-h.x, -h.y, h.z},	{h.x, -h.y, h.z},	 {h.x, h.y, h.z},	 {-h.x, h.y, h.z}};
		glm::vec3 firstWorldVertex = glm::vec3(finalTransform * glm::vec4(localVertices[0], 1.0f));
		worldAABB._min = worldAABB._max = firstWorldVertex;
		for (int i = 1; i < 8; ++i)
		{
			glm::vec3 worldVertex = glm::vec3(finalTransform * glm::vec4(localVertices[i], 1.0f));
			worldAABB._min = (glm::min)(worldAABB._min, worldVertex);
			worldAABB._max = (glm::max)(worldAABB._max, worldVertex);
		}
	}
	else if (shapeType == ShapeType::Mesh)
	{
		const auto* meshData = static_cast<const MeshShapeData*>(shape);
		auto& entity = meshData->entity;
		auto& vertices = entity.positions;
		auto& indices = entity.indices;
		if (!meshData || vertices.empty())
		{
			worldAABB._min = worldAABB._max = body.getPosition();
			return;
		}
		// Calculate AABB encompassing all transformed mesh vertices
		glm::vec3 firstWorldVertex = glm::vec3(finalTransform * glm::vec4(vertices[0], 1.0f));
		worldAABB._min = worldAABB._max = firstWorldVertex;
		for (size_t i = 1; i < vertices.size(); ++i)
		{
			glm::vec3 worldVertex = glm::vec3(finalTransform * glm::vec4(vertices[i], 1.0f));
			worldAABB._min = (glm::min)(worldAABB._min, worldVertex);
			worldAABB._max = (glm::max)(worldAABB._max, worldVertex);
		}
	}
	else
	{
		// Default for other shapes (e.g., sphere) or fallback
		worldAABB._min = worldAABB._max = body.getPosition(); // Placeholder
	}
}
/**
 * @brief Calculates the world-space Axis-Aligned Bounding Box (AABB)
 * that encloses the collider's volume swept over a given time interval.
 * @details This function predicts the collider's position and orientation at the
 * end of the time interval 'dt' based on its owner RigidBody's velocity.
 * It then computes the AABB at the start (t=0) and the predicted end (t=dt)
 * and merges them to create a conservative bounding box covering the motion.
 * This is crucial for CCD broadphase algorithms.
 * @param dt The time interval (delta time) over which to calculate the swept volume.
 * @return AABB The world-space AABB encompassing the collider's motion during dt.
 */
AABB Collider::getSweptWorldAABB(long double ldt) const
{
	auto body = ownerRigidBody;
	if (!body)
	{
		// If no owner body, return the static world AABB
		return getWorldAABB();
	}

	// --- 1. Calculate AABB at the start (t=0) ---
	AABB startAABB = getWorldAABB(); // Use existing method for current AABB

	// If the body is static or dt is negligible, the swept AABB is just the start AABB
	if (body->isStatic() || ldt <= 1e-9L)
	{
		return startAABB;
	}

	float dt = static_cast<float>(ldt); // Use float for glm calculations

	// --- 2. Predict Transform at the end (t=dt) ---

	// Predict final position
	// Simple Euler integration: finalPos = currentPos + linearVel * dt
	glm::vec3 currentPos = body->getPosition();
	glm::vec3 finalPos = currentPos + body->linearVelocity * dt;

	// Predict final orientation
	// Simple Euler integration for orientation: finalRot = deltaRot * currentRot
	glm::quat currentRot = body->getOrientation();
	glm::quat finalRot = currentRot;
	if (glm::length2(body->angularVelocity) > 1e-9f)
	{
		float angle = glm::length(body->angularVelocity) * dt;
		glm::vec3 axis = glm::normalize(body->angularVelocity);
		glm::quat rotationDelta = glm::angleAxis(angle, axis);
		finalRot = rotationDelta * currentRot;
		finalRot = glm::normalize(finalRot); // Ensure it remains normalized
	}

	// Construct the predicted world transform matrix for the RigidBody at t=dt
	glm::mat4 finalBodyTransform = glm::translate(glm::mat4(1.0f), finalPos) * glm::mat4_cast(finalRot);

	// --- 3. Calculate AABB at the predicted end (t=dt) ---

	// Combine predicted body transform with the collider's local offset/rotation
	glm::mat4 localOffsetTransform = glm::translate(glm::mat4(1.0f), getOffset()) * glm::mat4_cast(getRotationOffset());
	glm::mat4 finalColliderWorldTransform = finalBodyTransform * localOffsetTransform;

	// Calculate the AABB using the predicted final transform
	AABB endAABB = calculateWorldAABB(finalColliderWorldTransform);

	// --- 4. Merge Start and End AABBs ---
	AABB sweptAABB = AABB::merge(startAABB, endAABB);

	// --- 5. (Optional) Add a small margin for floating point errors ---
	// float epsilon = 0.01f; // Small padding value
	// sweptAABB.min -= glm::vec3(epsilon);
	// sweptAABB.max += glm::vec3(epsilon);

	return sweptAABB;
}

/**
 * @brief Calculates the world-space AABB for the collider given a specific world transform matrix.
 * @details This helper function takes a pre-calculated world transform matrix (which includes
 * body transform + collider offset/rotation) and computes the AABB based on the
 * collider's shape type and data.
 * @param worldTransform The specific world transform matrix to use for calculation.
 * @return AABB The calculated world-space AABB.
 */
AABB Collider::calculateWorldAABB(const glm::mat4& worldTransform) const
{
	AABB worldAABB; // Default constructor initializes to invalid/empty state
	auto shapeDataPtr = info.shapeData.get();

	if (!shapeDataPtr)
	{
		// Cannot calculate AABB without shape data
		std::cerr << "Warning: Cannot calculate AABB for collider without shape data." << std::endl;
		// Return an AABB centered at the transform's position with zero size?
		glm::vec3 pos = glm::vec3(worldTransform[3]);
		worldAABB._min = pos;
		worldAABB._max = pos;
		return worldAABB;
	}
	auto& shapeData = *shapeDataPtr;

	switch (shapeData.getType())
	{
	case ShapeType::Box:
		{
			const auto* boxData = static_cast<const BoxShapeData*>(shapeDataPtr);
			const glm::vec3 h = boxData->halfExtents; // Local half-extents

			// Define the 8 local vertices of the box
			glm::vec3 localVertices[8] = {{-h.x, -h.y, -h.z}, {h.x, -h.y, -h.z}, {h.x, h.y, -h.z}, {-h.x, h.y, -h.z},
																		{-h.x, -h.y, h.z},	{h.x, -h.y, h.z},	 {h.x, h.y, h.z},	 {-h.x, h.y, h.z}};

			// Transform each local vertex to world space and expand the AABB
			for (int i = 0; i < 8; ++i)
			{
				glm::vec4 worldVertex4 = worldTransform * glm::vec4(localVertices[i], 1.0f);
				worldAABB.encompass(glm::vec3(worldVertex4)); // Expand using the world vertex
			}
			break;
		}
	case ShapeType::Sphere:
		{
			const auto* sphereData = static_cast<const SphereShapeData*>(shapeDataPtr);
			float radius = sphereData->radius;
			glm::vec3 worldCenter = glm::vec3(worldTransform[3]); // Center is just the translation part

			// A sphere's AABB is simply its center +/- radius in each dimension
			worldAABB._min = worldCenter - glm::vec3(radius);
			worldAABB._max = worldCenter + glm::vec3(radius);
			break;
		}
	case ShapeType::Mesh:
		{
			const auto* meshData = static_cast<const MeshShapeData*>(shapeDataPtr);
			const auto& vertices = meshData->entity.positions; // Assuming positions are in local space

			if (vertices.empty())
			{
				std::cerr << "Warning: Cannot calculate AABB for mesh collider with no vertices." << std::endl;
				glm::vec3 pos = glm::vec3(worldTransform[3]);
				worldAABB._min = pos;
				worldAABB._max = pos;
				return worldAABB;
			}

			// Transform each mesh vertex to world space and expand the AABB
			for (const auto& localVertex : vertices)
			{
				glm::vec4 worldVertex4 = worldTransform * glm::vec4(localVertex, 1.0f);
				worldAABB.encompass(glm::vec3(worldVertex4));
			}
			break;
		}
	// TODO: Add cases for other shape types (Capsule, Cylinder, etc.)
	default:
		std::cerr << "Warning: AABB calculation not implemented for this shape type." << std::endl;
		glm::vec3 pos = glm::vec3(worldTransform[3]);
		worldAABB._min = pos;
		worldAABB._max = pos;
		break;
	}

	return worldAABB;
}

// Assume getWorldAABB() calls calculateWorldAABB with the *current* transform
AABB Collider::getWorldAABB() const
{
	auto body = ownerRigidBody;
	if (!body || !body->transform)
	{
		// Handle case where collider might exist without a body or transform (e.g., static collider)
		// Need a way to get its static world transform if applicable
		// For now, assume it needs a body+transform for non-static cases
		// If it's truly static, maybe store its world AABB directly?
		// Returning an empty/invalid AABB might be safer if state is unexpected.
		std::cerr << "Warning: Cannot get world AABB for collider without owner body/transform." << std::endl;
		return AABB(); // Return default (invalid) AABB
	}

	// Calculate current world transform of the collider
	auto& bodyWorldTransform = *body->transform; // Assuming transform component exists
	glm::mat4 localOffsetTransform = glm::translate(glm::mat4(1.0f), getOffset()) * glm::mat4_cast(getRotationOffset());
	glm::mat4 currentColliderWorldTransform = bodyWorldTransform * localOffsetTransform;

	return calculateWorldAABB(currentColliderWorldTransform);
}
ShapeType Collider::getShapeType() const { return info.shapeData ? info.shapeData->getType() : ShapeType::_Count; }
PhysicsMaterial& Collider::getPhysicsMaterial() { return info.material; }
const PhysicsMaterial& Collider::getPhysicsMaterial() const { return info.material; }
glm::vec3& Collider::getOffset() { return info.offset; }
const glm::vec3& Collider::getOffset() const { return info.offset; }
glm::quat& Collider::getRotationOffset() { return info.rotationOffset; }
const glm::quat& Collider::getRotationOffset() const { return info.rotationOffset; }
bool Collider::getIsSensor() { return info.isSensor; }
ColliderInfo& Collider::getColliderInfo() { return info; }
RigidBody* Collider::getOwnerRigidBody() { return ownerRigidBody; }
const glm::mat4* Collider::getTransform() { return transform; }
// const zg::AABB& Collider::getWorldAABB() const { return worldAABB; }

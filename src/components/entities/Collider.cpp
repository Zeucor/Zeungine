#include <zg/Entity.hpp>
#include <zg/Registry.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
#include <zg/physics/AABB.hpp>
using namespace zg::components::entities;
using zg::physics::AABB;
BoxShapeData::BoxShapeData(glm::vec3 halfExtents) : halfExtents(halfExtents) {}
JPH::ShapeRefC BoxShapeData::createJoltShape() const
{
	JPH::BoxShapeSettings settings(JPH::Vec3(halfExtents.x, halfExtents.y, halfExtents.z));
	// Optional: Add convex radius for better stability/performance trade-off
	// settings.mConvexRadius = 0.05f;
	auto result = settings.Create();
	if (result.HasError())
	{
		std::cerr << "Jolt ERROR creating BoxShape: " << result.GetError().c_str() << std::endl;
		return nullptr;
	}
	return result.Get();
}
JPH::ShapeRefC SphereShapeData::createJoltShape() const
{
	JPH::SphereShapeSettings settings(radius);
	auto result = settings.Create();
	if (result.HasError())
	{
		std::cerr << "Jolt ERROR creating SphereShape: " << result.GetError().c_str() << std::endl;
		return nullptr;
	}
	return result.Get();
}
MeshShapeData::MeshShapeData(Entity& entity) : entity(entity) {}
JPH::ShapeRefC MeshShapeData::createJoltShape() const
{
	std::cerr << "Jolt WARNING: MeshShape creation not implemented!" << std::endl;
	// Placeholder: Create a small box instead
	return BoxShapeData({0.1f, 0.1f, 0.1f}).createJoltShape();
	// --- Actual Implementation ---
	// 1. Get vertex and index data (e.g., from entity's MeshComponent)
	// JPH::VertexList vertices;
	// JPH::IndexedTriangleList triangles;
	// // ... populate vertices and triangles ...
	// JPH::MeshShapeSettings settings(vertices, triangles);
	// settings.SetEmbedded(); // Embed data in shape if desired
	// auto result = settings.Create();
	// if (result.HasError()) { /* handle error */ return nullptr; }
	// return result.Get();
}

ColliderInfo::ColliderInfo(const std::shared_ptr<ShapeData>& shapeData, const PhysicsMaterial& material, bool isSensor) :
		shapeData(shapeData), material(material), isSensor(isSensor) {};
ColliderInfo::ColliderInfo(const ColliderInfo& other) :
		shapeData(other.shapeData), material(other.material), isSensor(other.isSensor)
{
}
ColliderInfo& ColliderInfo::operator=(const ColliderInfo& other)
{
	shapeData = other.shapeData;
	material = other.material;
	isSensor = other.isSensor;
	return *this;
}
zg::components::entities::EntityComponentCreateInfo
zg::components::entities::ColliderFactory(const ColliderInfo& colliderInfo)
{
	zg::components::entities::EntityComponentCreateInfo createInfo{
		.name = "Collider",
		.onAttachedFunction = [colliderInfo](auto& component)
		{
			auto& entity = Registry::getEntity(component.HOST_INDEX_STACK);
			EntityComponent* rigidBodyComponentPointer = 0;
			try
			{
				rigidBodyComponentPointer = &entity.getComponentByName("RigidBody");
			}
			catch (...)
			{
				throw std::runtime_error("Collider Entity[" + std::to_string(entity.ID) + ", " + entity.name +
																	"] has no RigidBody component. You must add one before adding a Collider");
			}
			component.template make<bool>("IsSensor", colliderInfo.isSensor);
			rigidBodyComponentPointer->template setData<EntityComponent*>("attachCollider", &component);
		},
		.onDetachedFunction = [](auto& component)
		{
			auto& entity = Registry::getEntity(component.HOST_INDEX_STACK);
			EntityComponent* rigidBodyComponentPointer = 0;
			try
			{
				rigidBodyComponentPointer = &entity.getComponentByName("RigidBody");
			}
			catch (...)
			{
				throw std::runtime_error("Collider Entity[" + std::to_string(entity.ID) + ", " + entity.name +
																	"] has no RigidBody component. You must add one before adding a Collider");
			}
			rigidBodyComponentPointer->template setData<EntityComponent*>("detachCollider", &component);
		},
		.onUpdateFunction = [](auto& component)
		{

		},
		.getDataFunctions = {
			{"Info", [colliderInfo](auto& component)->std::any&
			{
				try
				{
					return component.getDataReturnAny("m_Info");
				}
				catch (...)
				{
					return component.template makeReturnAny<ColliderInfo>("m_Info", colliderInfo);
				}
			}},
			{"PhysicsMaterial", [colliderInfo](auto& component)->std::any&
			{
				try
				{
					return component.getDataReturnAny("m_PhysicsMaterial");
				}
				catch (...)
				{
					return component.template makeReturnAny<PhysicsMaterial>("m_PhysicsMaterial", colliderInfo.material);
				}
			}},
		}
	};
	return createInfo;
}
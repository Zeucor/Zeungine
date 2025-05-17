#include <zg/Entity.hpp>
#include <zg/Registry.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
#include <zg/physics/AABB.hpp>
#undef min
#undef max
#define ENABLE_VHACD_IMPLEMENTATION 1
#include <VHACD.h>
using namespace zg::components::entities;
using zg::physics::AABB;
BoxShapeData::BoxShapeData(glm::vec3 halfExtents) : halfExtents(halfExtents) {}
JPH::ShapeRefC BoxShapeData::createJoltShape(Entity& entity) const
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
JPH::ShapeRefC SphereShapeData::createJoltShape(Entity& entity) const
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
JPH::ShapeRefC MeshShapeData::createJoltShape(Entity& entity) const
{
	JPH::StaticCompoundShapeSettings compoundSettings;
	for (auto& meshID : entity.meshIDs)
	{
		auto& mesh = Registry::getMesh(meshID);
		auto& entityVertices = mesh.vertices;
		auto& entityIndices = mesh.indices;
		if (entityIndices.size() % 3 != 0)
		{
			std::cerr << "Error: Index count (" << entityIndices.size() << ") is not a multiple of 3 for entity " << entity.ID << std::endl;
			return nullptr;
		}
		JPH::VertexList vertices;
		vertices.reserve(entityVertices.size());
		for (const auto& v : entityVertices)
		{
			vertices.emplace_back(v.x, v.y, v.z);
		}
		JPH::IndexedTriangleList triangles;
		size_t numTriangles = entityIndices.size() / 3;
		triangles.reserve(numTriangles);
		for (size_t i = 0; i < entityIndices.size(); i += 3)
		{
			auto i0 = entityIndices[i];
			auto i1 = entityIndices[i + 1];
			auto i2 = entityIndices[i + 2];
			if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size())
			{
				 std::cerr << "Error: Vertex index out of bounds for entity " << entity.ID << std::endl;
				 return nullptr;
			}
			triangles.emplace_back(i0, i1, i2, 0);
		}
		JPH::MeshShapeSettings settings(vertices, triangles);
		settings.SetEmbedded();
		auto result = settings.Create();
		if (result.HasError())
		{
			std::cerr << "Error creating Jolt MeshShape for entity " << entity.ID << ": " << result.GetError().c_str() << std::endl;
			return nullptr;
		}
		auto shape = result.Get();
		compoundSettings.AddShape(
			JPH::Vec3(0.0f, 0.0f, 0.0f),
			JPH::Quat::sIdentity(),
			shape
		);
	}
    JPH::Shape::ShapeResult compoundShapeResult = compoundSettings.Create();
    if (compoundShapeResult.HasError())
	{
         std::cout << "Error creating compound shape: " << compoundShapeResult.GetError().c_str() << std::endl;
        return nullptr;
    }
    return compoundShapeResult.Get();
}

JPH::ShapeRefC ConvexHullShapeData::createJoltShape(Entity& entity) const
{
	JPH::StaticCompoundShapeSettings compoundSettings;
	for (auto& meshID : entity.meshIDs)
	{
		auto& mesh = Registry::getMesh(meshID);
		std::vector<glm::vec3> entityVertices = mesh.vertices;
		if (entityVertices.empty())
		{
			std::cerr << "Error: Cannot create convex hull with zero vertices." << std::endl;
			return nullptr;
		}
		if (entityVertices.size() < 4)
		{
			std::cerr << "Warning: Trying to create a convex hull with fewer than 4 vertices (" << entityVertices.size() << ")." << std::endl;
			return nullptr;
		}
		std::vector<uint32_t> entityIndices = mesh.indices;
		std::vector<double> vhacdVertices;
		vhacdVertices.reserve(entityVertices.size() * 3);
		for (auto& v : entityVertices)
		{
			vhacdVertices.push_back(v.x);
			vhacdVertices.push_back(v.y);
			vhacdVertices.push_back(v.z);
		}
		VHACD::IVHACD* vhacd = VHACD::CreateVHACD();
		VHACD::IVHACD::Parameters params;
		params.m_resolution = 250;
		params.m_minimumVolumePercentErrorAllowed = 0.1;
		params.m_maxRecursionDepth = 5;
		params.m_maxConvexHulls = 10;
		params.m_maxNumVerticesPerCH = 1024;
		// params.m_minimumVolumePercentErrorAllowed = 0.1;       // Maximum concavity
		// params.m_maxNumVerticesPerCH = 256; // Max vertices per convex hull
		// params.m_resolution = 100000;    // Voxelization resolution
		// params.m_pca = false;
		// params.m_planeDownsampling = 4;
		bool success = vhacd->Compute(
			vhacdVertices.data(),
			static_cast<unsigned int>(vhacdVertices.size() / 3),
			entityIndices.data(),
			static_cast<unsigned int>(entityIndices.size() / 3),
			params
		);
		if (!success)
		{
			vhacd->Release();
			throw std::runtime_error("Error: VHACD decomposition failed");
		}
		auto numConvexHulls = vhacd->GetNConvexHulls();
		for (unsigned int i = 0; i < numConvexHulls; ++i)
		{
			VHACD::IVHACD::ConvexHull ch;
			vhacd->GetConvexHull(i, ch);
			JPH::Array<JPH::Vec3> joltVertices;
			auto chPointsSize = ch.m_points.size();
			auto chPointsData = ch.m_points.data();
			joltVertices.reserve(chPointsSize);
			for (size_t j = 0; j < chPointsSize; ++j)
			{
				auto& v = chPointsData[j];
				joltVertices.push_back(JPH::Vec3(v.mX, v.mY, v.mZ));
			}
			JPH::ConvexHullShapeSettings settings(joltVertices);
			auto result = settings.Create();
			if (result.HasError())
			{
				std::cerr << "Error creating Jolt ConvexHullShape: " << result.GetError().c_str() << std::endl;
				return nullptr;
			}
			auto shape = result.Get();
			compoundSettings.AddShape(
				JPH::Vec3(0.0f, 0.0f, 0.0f),
				JPH::Quat::sIdentity(),
				shape
			);
		}
		vhacd->Release();
	}
    JPH::Shape::ShapeResult compoundShapeResult = compoundSettings.Create();
    if (compoundShapeResult.HasError())
	{
         std::cout << "Error creating compound shape: " << compoundShapeResult.GetError().c_str() << std::endl;
        return nullptr;
    }
    return compoundShapeResult.Get();
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
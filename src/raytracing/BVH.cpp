#include <iostream>
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/raytracing/BVH.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/entities/Cube.hpp>
using namespace zg::raytracing;
static constexpr bool shouldPermute = true;
BVH::BVH() : executor(threadPool), config(getDefaultConfig()) {}
Config BVH::getDefaultConfig()
{
	Config config;
	config.quality = bvh::v2::DefaultBuilder<Node>::Quality::High;
	return config;
}
void BVH::buildBBoxesAndCenters()
{
	bboxes.resize(triangles.size());
	centers.resize(triangles.size());
	executor.for_each(0, triangles.size(),
										[&](size_t begin, size_t end)
										{
											for (size_t i = begin; i < end; ++i)
											{
												bboxes[i] = triangles[i].get_bbox();
												centers[i] = triangles[i].get_center();
											}
										});
}
void BVH::buildBVH()
{
	if (!triangles.size())
	{
		return;
	}
	buildBBoxesAndCenters();
	bvh = Builder::build(threadPool, bboxes, centers, config);
	precomputeTriangles();
	changed = false;
	built = true;
}
void BVH::precomputeTriangles()
{
	precomputedTriangles.resize(triangles.size());
	executor.for_each(0, triangles.size(),
										[&](size_t begin, size_t end)
										{
											for (size_t i = begin; i < end; ++i)
											{
												auto j = shouldPermute ? bvh.prim_ids[i] : i;
												precomputedTriangles[i] = triangles[j];
											}
										});
}
size_t BVH::trace(Ray& ray)
{
	if (!built || changed)
	{
		buildBVH();
	}
	if (!built)
	{
		return invalidID;
	}
	auto primID = invalidID;
	Scalar u, v;
	bvh::v2::SmallStack<Bvh::Index, stackSize> stack;
	bvh.intersect<false, useRobustTraversal>(ray, bvh.get_root().index, stack,
																					 [&](size_t begin, size_t end)
																					 {
																						 for (size_t i = begin; i < end; ++i)
																						 {
																							 size_t j = shouldPermute ? i : bvh.prim_ids[i];
																							 if (auto hit = precomputedTriangles[j].intersect(ray))
																							 {
																								 primID = i;
																								 std::tie(ray.tmax, u, v) = *hit;
																							 }
																						 }
																						 return primID != invalidID;
																					 });
	return primID;
}
size_t BVH::addTriangle(const Tri& tri)
{
	auto triangleID = triangles.size();
	triangles.push_back(tri);
	changed = true;
	built = false;
	return triangleID;
}
void BVH::addEntity(Entity& entity)
{
	if (entity.addToBVH)
	{
		size_t meshIndex = 0;
		for (auto& meshID : entity.meshIDs)
		{
			auto& mesh = Registry::GetSingleton().getMesh(meshID);
			auto& m_indices = mesh.indices;
			auto& m_vertices = mesh.vertices;
			std::vector<uint32_t> t_indices;
			std::vector<glm::vec3> t_vertices;
			auto p_indices = &m_indices;
			auto p_vertices = &m_vertices;
			if (m_indices.empty())
			{
				if (mesh.info.shapeType >= ShapeType::PlaneXZ_Center && mesh.info.shapeType <= ShapeType::PlaneXY_BottomLeft)
				{
					t_indices = entities::getPlaneIndices((entities::PlaneType)mesh.info.shapeType);
					t_vertices = entities::getPlaneVertices((entities::PlaneType)mesh.info.shapeType);
				}
				else if (mesh.info.shapeType == ShapeType::Box)
				{
					t_indices = entities::getCubeIndices();
					t_vertices = entities::getCubeVertices();
				}
				p_indices = &t_indices;
				p_vertices = &t_vertices;
			}
			auto& indices = *p_indices;
			auto& vertices = *p_vertices;
			auto indices_data = indices.data();
			auto vertices_data = vertices.data();
			auto& model = entity.getModelMatrix();
			auto indiceCount = indices.size();
			for (size_t i = 0, c = 0; i < indiceCount; c++, i += 3)
			{
				auto i0 = indices_data[i + 0];
				auto i1 = indices_data[i + 1];
				auto i2 = indices_data[i + 2];
				auto v0 = vertices_data[i0];
				auto v1 = vertices_data[i1];
				auto v2 = vertices_data[i2];
				v0 = glm::vec3(model * glm::vec4(v0, 1.0f));
				v1 = glm::vec3(model * glm::vec4(v1, 1.0f));
				v2 = glm::vec3(model * glm::vec4(v2, 1.0f));
				addTriangle({{v0.x, v0.y, v0.z}, {v1.x, v1.y, v1.z}, {v2.x, v2.y, v2.z}, {entity.ID, meshIndex}});
			}
			meshIndex++;
		}
	}
}
void BVH::updateEntity(Entity& entity)
{
	if (entity.addToBVH)
	{
		size_t meshIndex = 0;
		for (auto& meshID : entity.meshIDs)
		{
			auto& mesh = Registry::GetSingleton().getMesh(meshID);
			std::vector<size_t> indices;
			size_t indicesCount = 0;
			auto trianglesSize = triangles.size();
			for (size_t i = 0; i < trianglesSize; ++i)
			{
				if (triangles[i].userData.first == entity.ID && triangles[i].userData.second == meshIndex)
				{
					++indicesCount;
				}
			}
			indices.reserve(indicesCount);
			for (size_t i = 0; i < trianglesSize; ++i)
			{
				if (triangles[i].userData.first == entity.ID && triangles[i].userData.second == meshIndex)
				{
					indices.push_back(i);
				}
			}
			auto indiceCount = mesh.indices.size();
			auto& m_indices = mesh.indices;
			auto& m_vertices = mesh.vertices;
			std::vector<uint32_t> t_indices;
			std::vector<glm::vec3> t_vertices;
			auto p_indices = &m_indices;
			auto p_vertices = &m_vertices;
			if (m_indices.empty())
			{
				if (mesh.info.shapeType >= ShapeType::PlaneXZ_Center && mesh.info.shapeType <= ShapeType::PlaneXY_BottomLeft)
				{
					t_indices = entities::getPlaneIndices((entities::PlaneType)mesh.info.shapeType);
					t_vertices = entities::getPlaneVertices((entities::PlaneType)mesh.info.shapeType);
				}
				else if (mesh.info.shapeType == ShapeType::Box)
				{
					t_indices = entities::getCubeIndices();
					t_vertices = entities::getCubeVertices();
				}
				p_indices = &t_indices;
				p_vertices = &t_vertices;
			}
			auto& meshIndices = *p_indices;
			auto& meshVertices = *p_vertices;
			if (meshIndices.empty())
				return;
			auto indices_size = meshIndices.size();
			auto indices_data = meshIndices.data();
			auto vertices_data = meshVertices.data();
			auto& model = entity.getModelMatrix();
			for (size_t i = 0, c = 0; i < indices_size; c++, i += 3)
			{
				auto& triangleID = indices[c];
				auto i0 = indices_data[i + 0];
				auto i1 = indices_data[i + 1];
				auto i2 = indices_data[i + 2];
				auto v0 = vertices_data[i0];
				auto v1 = vertices_data[i1];
				auto v2 = vertices_data[i2];
				v0 = glm::vec3(model * glm::vec4(v0, 1.0f));
				v1 = glm::vec3(model * glm::vec4(v1, 1.0f));
				v2 = glm::vec3(model * glm::vec4(v2, 1.0f));
				triangles[triangleID] = {{v0.x, v0.y, v0.z}, {v1.x, v1.y, v1.z}, {v2.x, v2.y, v2.z}, {entity.ID, meshIndex}};
			}
			meshIndex++;
		}
	}
	built = false;
	changed = true;
}
void BVH::removeEntity(Scene& scene, Entity& entity)
{
	if (entity.addToBVH)
	{
		size_t removalIndex = 0;
		auto entityID = entity.ID;
		triangles.erase(std::remove_if(triangles.begin(), triangles.end(),
																	 [&](const Tri& tri) -> bool
																	 {
																		 if (tri.userData.first == entityID)
																		 {
																			 removalIndex++;
																			 return true;
																		 }
																		 return false;
																	 }),
										triangles.end());
	}
	changed = true;
	built = false;
}

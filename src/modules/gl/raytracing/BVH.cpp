#include <anex/modules/gl/raytracing/BVH.hpp>
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
using namespace anex::modules::gl::raytracing;
static constexpr bool shouldPermute = true;
BVH::BVH():
	executor(threadPool),
  config(getDefaultConfig())
{

};
Config BVH::getDefaultConfig()
{
  Config config;
	config.quality = bvh::v2::DefaultBuilder<Node>::Quality::High;
  return config;
};
void BVH::buildBBoxesAndCenters()
{
	bboxes.resize(triangles.size());
	centers.resize(triangles.size());
	executor.for_each(0, triangles.size(), [&] (size_t begin, size_t end)
  {
		for (size_t i = begin; i < end; ++i)
    {
			bboxes[i]  = triangles[i].get_bbox();
			centers[i] = triangles[i].get_center();
		}
	});
};
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
};
void BVH::precomputeTriangles()
{
  precomputedTriangles.resize(triangles.size());
	executor.for_each(0, triangles.size(), [&] (size_t begin, size_t end)
  {
		for (size_t i = begin; i < end; ++i)
    {
			auto j = shouldPermute ? bvh.prim_ids[i] : i;
			precomputedTriangles[i] = triangles[j];
		}
	});
};
glm::vec3 BVH::unProject(const glm::vec3 &win, const glm::mat4 &inverseProjectionView, const glm::vec4 &viewport)
{
	glm::vec4 tmp = glm::vec4(win, 1.0f);
	// Convert screen coordinates to NDC
	tmp.x = (tmp.x - viewport[0]) / viewport[2] * 2.0f - 1.0f;
	tmp.y = (tmp.y - viewport[1]) / viewport[3] * 2.0f - 1.0f;
	tmp.z = win.z * 2.0f - 1.0f;  // Map win.z from [0, 1] to [-1, 1]

	glm::vec4 obj = inverseProjectionView * tmp;  // Transform into world space
	obj /= obj.w;  // Perform perspective divide
	return glm::vec3(obj);
};
Ray BVH::mouseCoordToRay(const uint32_t &windowHeight,
                         glm::vec2 screenCoord,
                         const glm::vec4 &viewport,
                         const glm::mat4 &projection,
                         const glm::mat4 &view,
                         const float &nearPlane,
                         const float &farPlane)
{
	glm::mat4 inverseProjectionView = glm::inverse(projection * view);
	glm::vec3 nearPoint = unProject(glm::vec3(screenCoord, 0.0), inverseProjectionView, viewport);
	glm::vec3 farPoint = unProject(glm::vec3(screenCoord, 0.99999), inverseProjectionView, viewport);
	glm::vec3 rayDir = glm::normalize(farPoint - nearPoint);
	Ray ray;
	ray.org[0] = nearPoint.x;
	ray.org[1] = nearPoint.y;
	ray.org[2] = nearPoint.z;
	ray.dir[0] = rayDir.x;
	ray.dir[1] = rayDir.y;
	ray.dir[2] = rayDir.z;
	ray.tmin = nearPlane;
	ray.tmax = farPlane;
  return ray;
};
size_t BVH::trace(Ray &ray)
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
	bvh.intersect<false, useRobustTraversal>(ray, bvh.get_root().index, stack, [&] (size_t begin, size_t end)
  {
		for (size_t i = begin; i < end; ++i)
    {
			size_t j = shouldPermute ? i : bvh.prim_ids[i];
			if (auto hit = precomputedTriangles[j].intersect(ray))
      {
				primID = i;
				std::tie(u, v) = *hit;
			}
		}
		return primID != invalidID;
	});
  return primID;
};
size_t BVH::addTriangle(const Tri &tri)
{
  auto triangleID = triangles.size();
  triangles.push_back(tri);
  changed = true;
	built = false;
  return triangleID;
};
std::vector<size_t> BVH::addEntity(GLEntity &entity)
{
  auto &indiceCount = entity.indiceCount;
  auto indicesData = entity.indices.data();
  auto verticesData = entity.positions.data();
  auto &model = entity.getModelMatrix();
	std::vector<size_t> triangleIDs(indiceCount / 3);
	for (size_t i = 0, c = 0; i < indiceCount; c++, i += 3)
	{
		auto i0 = indicesData[i + 0];
		auto i1 = indicesData[i + 1];
		auto i2 = indicesData[i + 2];
		auto v0 = verticesData[i0];
		auto v1 = verticesData[i1];
		auto v2 = verticesData[i2];
		v0 = glm::vec3(model * glm::vec4(v0, 1.0f));
		v1 = glm::vec3(model * glm::vec4(v1, 1.0f));
		v2 = glm::vec3(model * glm::vec4(v2, 1.0f));
		triangleIDs[c] = addTriangle({
			{v0.x, v0.y, v0.z},
			{v1.x, v1.y, v1.z},
			{v2.x, v2.y, v2.z}
		});
	}
	entity.triangleIDs = triangleIDs;
	return triangleIDs;
};
void BVH::updateEntity(GLEntity &entity)
{
	auto &triangleIDs = entity.triangleIDs;
	auto &indiceCount = entity.indiceCount;
	auto indicesData = entity.indices.data();
	auto verticesData = entity.positions.data();
	auto &model = entity.getModelMatrix();
	for (size_t i = 0, c = 0; i < indiceCount; c++, i += 3)
	{
		auto &triangleID = triangleIDs[c];
		auto i0 = indicesData[i + 0];
		auto i1 = indicesData[i + 1];
		auto i2 = indicesData[i + 2];
		auto v0 = verticesData[i0];
		auto v1 = verticesData[i1];
		auto v2 = verticesData[i2];
		v0 = glm::vec3(model * glm::vec4(v0, 1.0f));
		v1 = glm::vec3(model * glm::vec4(v1, 1.0f));
		v2 = glm::vec3(model * glm::vec4(v2, 1.0f));
		triangles[triangleID] = {
			{v0.x, v0.y, v0.z},
			{v1.x, v1.y, v1.z},
			{v2.x, v2.y, v2.z}
		};
	}
	built = false;
	changed = true;
};
void BVH::removeEntity(GLScene &scene, GLEntity &entity)
{
	auto triangleIDs = entity.triangleIDs;
	size_t removalIndex = 0;
	triangles.erase(std::remove_if(triangles.begin(), triangles.end(), [&](const Tri &tri) -> bool
	{
		if (removalIndex < triangleIDs.size() &&
			 (&tri - &triangles[0]) == triangleIDs[removalIndex])
		{
			removalIndex++;
			return true; // Mark for removal
		}
		return false;
	}), triangles.end());
	entity.triangleIDs.clear();
	auto triangleIDsSize = triangleIDs.size();
	if (triangleIDsSize)
	{
		auto maxTriangleID = triangleIDs.back();
		std::vector<std::pair<size_t, std::vector<size_t>>> updatedPairs;
		for (auto &pair : scene.triangleIDsToEntityIDsMap)
		{
			if (pair.first > maxTriangleID)
			{
				updatedPairs.emplace_back(pair.first - triangleIDsSize, pair.second);
			}
		}
		for (auto &pair : updatedPairs)
		{
			scene.triangleIDsToEntityIDsMap.erase(pair.first + triangleIDsSize);
		}
		for (auto &pair : updatedPairs)
		{
			scene.triangleIDsToEntityIDsMap.emplace(pair.first, pair.second);
		}
		for (auto &triangleID : triangleIDs)
		{
			scene.triangleIDsToEntityIDsMap.erase(triangleID);
		}
		scene.adjustTriangleIDs(triangleIDs.front(), triangleIDsSize);
	}
	for (auto &pair : entity.children)
	{
		removeEntity(scene, *pair.second);
	}
	changed = true;
	built = false;
};
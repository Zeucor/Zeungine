#include <anex/modules/gl/raytracing/BVH.hpp>
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
void BVH::buildBVH()
{
  precomputeTriangles();
	bvh = Builder::build(threadPool, bboxes, centers, config);
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
	glm::vec4 tmp = glm::vec4(win, 1);
	tmp.x = (tmp.x - viewport[0]) / viewport[2];
	tmp.y = (tmp.y - viewport[1]) / viewport[3];
	tmp.x = tmp.x * 2 - 1;
	tmp.y = tmp.y * 2 - 1;
	glm::vec4 obj = inverseProjectionView * tmp;
	obj /= obj.w;
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
	screenCoord.y = windowHeight - screenCoord.y;
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
#pragma once
#include <bvh/v2/bvh.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/ray.h>
#include <bvh/v2/node.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/thread_pool.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/stack.h>
#include <bvh/v2/tri.h>
#include <anex/glm.hpp>
namespace anex::modules::gl::raytracing
{
	using Scalar = float;
	using Vec3 = bvh::v2::Vec<Scalar, 3>;
	using BBox = bvh::v2::BBox<Scalar, 3>;
	using Tri = bvh::v2::Tri<Scalar, 3>;
	using Node = bvh::v2::Node<Scalar, 3>;
	using Bvh = bvh::v2::Bvh<Node>;
	using Ray = bvh::v2::Ray<Scalar, 3>;
	using ThreadPool = bvh::v2::ThreadPool;
	using ParallelExecutor = bvh::v2::ParallelExecutor;
  using Config = bvh::v2::DefaultBuilder<Node>::Config;
	using PrecomputedTri = bvh::v2::PrecomputedTri<Scalar>;
  using Builder = bvh::v2::DefaultBuilder<Node>;
	struct BVH
  {
		std::vector<Tri> triangles;
		std::vector<PrecomputedTri> precomputedTriangles;
		ThreadPool threadPool;
		ParallelExecutor executor;
		std::vector<BBox> bboxes;
		std::vector<Vec3> centers;
		Config config;
    Bvh bvh;
    BVH();
    Config getDefaultConfig();
    void buildBVH();
    void precomputeTriangles();
		glm::vec3 unProject(const glm::vec3 &win, const glm::mat4 &inverseProjectionView, const glm::vec4 &viewport);
    Ray mouseCoordToRay(const uint32_t &windowHeight,
                        glm::vec2 screenCoord,
												const glm::vec4 &viewport,
												const glm::mat4 &projection,
												const glm::mat4 &view,
												const float &nearPlane,
												const float &farPlane);
  };
}
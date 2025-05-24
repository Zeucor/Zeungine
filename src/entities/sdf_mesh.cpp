#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "MC33.h"
// #include <zg/CGAL.hpp>
#include <zg/entities/SDF.hpp>
#include <zg/entities/sdf_mesh.hpp>
#include <zg/glm.hpp>
#include <zg/system/Budget.hpp>
using namespace zg;
using namespace zg::entities;
void generate_mesh_from_sdf(const Entity& entity,
							std::vector<glm::vec3>& out_vertices,
							std::vector<uint32_t>& out_indices)
{
	grid3d G;
	G.generate_grid_from_fn(-2.0, -2.0, -2.0, 2.0, 2.0, 2.0, 0.1, 0.1, 0.1, [&](double x, double y, double z)  -> double {
		return entity(glm::vec3(x, y, z));
	});
	MC33 MC;
  	MC.set_grid3d(G);
	surface S;
	MC.calculate_isosurface(S, 0.0);
	auto vertices_count = S.get_num_vertices();
	auto triangle_count = S.get_num_triangles();
	std::unordered_map<glm::vec3, uint32_t> vertex_to_index;
	vertex_to_index.reserve(vertices_count);
    out_vertices.reserve(vertices_count);
    out_indices.reserve(triangle_count * 3);
    std::array<uint32_t, 3> final_indices;
    std::array<uint32_t, 3> triangle_indices;
	std::array<glm::vec3, 3> triangle;
	for (size_t i = 0; i < triangle_count; i++)
	{
		auto tri = S.getTriangle(i);
		auto v_0 = (glm::vec3*)S.getVertex(tri[0]);
		auto v_1 = (glm::vec3*)S.getVertex(tri[1]);
		auto v_2 = (glm::vec3*)S.getVertex(tri[2]);
		triangle = { *v_0, *v_1, *v_2 };
		for (int i = 0; i < 3; ++i)
		{
			auto iter = vertex_to_index.find(triangle[i]);
			if (iter == vertex_to_index.end())
			{
				uint32_t idx = (uint32_t)out_vertices.size();
				out_vertices.push_back(triangle[i]);
				vertex_to_index[triangle[i]] = idx;
				triangle_indices[i] = idx;
			}
			else
			{
				triangle_indices[i] = iter->second;
			}
		}
		final_indices[0] = triangle_indices[0];
		final_indices[1] = triangle_indices[1];
		final_indices[2] = triangle_indices[2];
        out_indices.insert(out_indices.end(), final_indices.begin(), final_indices.end());
	}
	return;
}
EntityCreateInfo zg::entities::sdf_mesh_factory(const std::string& sdf_key, const std::string& name, glm::vec3 position,
																								glm::quat rotation, glm::vec3 scale, glm::vec4 color,
																								const shaders::RuntimeConstants& constants)
{
	auto sdf_type = SDFRegistry::GetSingleton().get_sdf_type(sdf_key);
	MeshCreateInfo meshInfo{
        .name = sdf_key,
        .shapeType = ShapeType::Mesh,
        .material = {color, 0},
        .info = [](auto& entity) -> MeshInfo
        {
            auto& color = entity.template getData<glm::vec4>("Color");
            std::vector<glm::vec3> vertices;
            std::vector<uint32_t> indices;
            auto before = SYS_CLOCK::now();
            generate_mesh_from_sdf(entity,/*entity.getSuggestedBoundingSphere(),*/ vertices, indices);
            auto after = SYS_CLOCK::now();
            auto diff_count = (after - before).count();
            // diff_dur
            // diff_dur
            std::cout << "generated SDF mesh in: " << (diff_count / 10'000'000.0L) << "s" << std::endl;
            return {
                .indices = indices,
                .vertices = vertices
            };
        },
        .constants = shaders::mergeConstants(
            {shaders::RuntimeConstants({"Shape", "Color"}), shaders::common_zg_constants, constants}),
        .meta_int = sdf_type
    };
	EntityCreateInfo info{
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .name = sdf_key,
        .dataMap = {
            {"Color", color}
        },
        .meshInfos = {meshInfo}
    };
	return info;
}

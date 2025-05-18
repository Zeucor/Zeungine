#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <zg/CGAL.hpp>
#include <zg/entities/SDF.hpp>
#include <zg/entities/sdf_mesh.hpp>
#include <zg/glm.hpp>
#include <zg/system/Budget.hpp>
using namespace zg;
using namespace zg::entities;
namespace params = CGAL::parameters;
template <typename SDF_Functor_Type>
void generate_mesh_from_sdf(const SDF_Functor_Type& sdf_functor, const K::Sphere_3& cgal_bounding_sphere,
														std::vector<glm::vec3>& out_vertices, std::vector<uint32_t>& out_indices)
{
	Mesh_domain domain = Mesh_domain::create_implicit_mesh_domain(sdf_functor, cgal_bounding_sphere);
	Mesh_criteria criteria(
		params::facet_angle(30).facet_size(0.025).facet_distance(0.50).cell_radius_edge_ratio(2.5).cell_size(0.2));

	C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria);

	std::unordered_map<glm::vec3, uint32_t> vertex_to_index;
    auto vertices_count = c3t3.number_of_vertices_in_complex();
	vertex_to_index.reserve(vertices_count);
    out_vertices.reserve(vertices_count);
    out_indices.reserve(c3t3.number_of_facets_in_complex() * 3);

	auto to_glm = [](const auto& p) -> glm::vec3 { return glm::vec3((float)p.x(), (float)p.y(), (float)p.z()); };

	auto to_cgal = [](const auto& p) -> K::Point_3 { return K::Point_3(p.x, p.y, p.z); };

    std::array<uint32_t, 3> triangle_indices;
    std::array<uint32_t, 3> final_indices;
    std::array<Vertex_handle, 4> vh;
    std::array<int, 3> indices;
	for (auto fit = c3t3.facets_in_complex_begin(); fit != c3t3.facets_in_complex_end(); ++fit)
	{
		auto cell = fit->first;
		int opposite = fit->second;

		for (int i = 0; i < 4; ++i)
		{
			vh[i] = cell->vertex(i);
		}

		for (int i = 0, j = 0; i < 4; ++i)
		{
			if (i != opposite)
			{
				indices[j++] = i;
			}
		}

		auto p0 = to_glm(vh[indices[0]]->point());
		auto p1 = to_glm(vh[indices[1]]->point());
		auto p2 = to_glm(vh[indices[2]]->point());

		auto centroid = (p0 + p1 + p2) / 3.0f;
		auto normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
		auto sdf_grad = glm::normalize(glm::vec3(
			(sdf_functor(to_cgal(centroid + glm::vec3(1e-3f, 0, 0))) - sdf_functor(to_cgal(centroid - glm::vec3(1e-3f, 0, 0)))) /
				(2e-3f),
			(sdf_functor(to_cgal(centroid + glm::vec3(0, 1e-3f, 0))) - sdf_functor(to_cgal(centroid - glm::vec3(0, 1e-3f, 0)))) /
				(2e-3f),
			(sdf_functor(to_cgal(centroid + glm::vec3(0, 0, 1e-3f))) - sdf_functor(to_cgal(centroid - glm::vec3(0, 0, 1e-3f)))) /
				(2e-3f)));

		bool flip = glm::dot(normal, sdf_grad) < 0.0f;

		std::array<glm::vec3, 3> triangle = {p0, p1, p2};

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

		if (flip)
		{
			final_indices[0] = triangle_indices[1];
			final_indices[1] = triangle_indices[2];
			final_indices[2] = triangle_indices[0];
		}
		else
		{
			final_indices[0] = triangle_indices[2];
			final_indices[1] = triangle_indices[1];
			final_indices[2] = triangle_indices[0];
		}
        out_indices.insert(out_indices.end(), final_indices.begin(), final_indices.end());
	}
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
            generate_mesh_from_sdf(entity, entity.get_suggested_bounding_sphere(), vertices, indices);
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

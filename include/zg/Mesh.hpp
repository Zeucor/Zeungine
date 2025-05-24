#pragma once
#include <zg/vaos/VAO.hpp>
#include <zg/Shape.hpp>
#include <zg/Material.hpp>
#include <zg/tracy.hpp>
namespace zg
{
    struct MeshCreateInfo;
	struct MeshInfo
	{
        size_t hash;
		std::vector<uint32_t> indices;
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec4> colors;
		std::vector<glm::vec2> uv2s;
		std::vector<glm::vec3> uv3s;
	};
    struct MeshCreateInfo
    {
        size_t hash;
		std::string name;
		ShapeType shapeType;
		Material material;
		std::function<MeshInfo(const Entity&)> info;
		std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> keyedTextures;
		shaders::RuntimeConstants constants;
		int32_t meta_int = -1;
		std::function<float(const Entity&, glm::vec3)> sdf_c_function;
		std::unordered_map<size_t, MeshInfo> entity_id_mesh_infos;
    };
    struct Mesh : MeshInfo, vaos::VAO
    {
		size_t ID = 0;
		size_t* INDEX = 0;
		std::vector<size_t*> INDEX_STACK;
		MeshCreateInfo info;
		bool setTexturesThisPass = false;
        Mesh(const MeshCreateInfo& info, Entity& entity);
        Mesh(const Mesh& other);
        Mesh& operator=(const Mesh& other);
		uint32_t getIndicesSize(ShapeType shapeType);
		uint32_t getVerticesSize(ShapeType shapeType);
        void render(Entity& entity);
		template <typename T>
		static void flipUVsY(std::vector<T>& uvs)
		{
			for (auto& uv : uvs)
			{
				uv.y = 1 - uv.y;
			}
		}
    };
}
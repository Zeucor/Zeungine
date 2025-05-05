#pragma once
#include <zg/vaos/VAO.hpp>
namespace zg
{
    struct MeshCreateInfo;
    struct Mesh : vaos::VAO
    {
		size_t ID = 0;
		size_t* INDEX = 0;
		std::vector<size_t*> INDEX_STACK;
        size_t hash;
		std::function<uint32_t(Entity&)> indiceCount;
		std::function<std::vector<uint32_t>(Entity&)> indices;
		std::function<uint32_t(Entity&)> vertexCount;
		std::function<std::vector<glm::vec3>(Entity&)> vertices;
		std::function<uint32_t(Entity&)> colorCount;
		std::function<std::vector<glm::vec4>(Entity&)> colors;
		std::function<uint32_t(Entity&)> uv2Count;
		std::function<std::vector<glm::vec2>(Entity&)> uv2s;
		std::function<uint32_t(Entity&)> uv3Count;
		std::function<std::vector<glm::vec3>(Entity&)> uv3s;
		std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> keyedTextures;
		bool setTexturesThisPass = false;
        Mesh(const MeshCreateInfo& info, Entity& entity);
        Mesh(const Mesh& other);
        Mesh& operator=(const Mesh& other);
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
    struct MeshCreateInfo
    {
        size_t hash;
		std::function<uint32_t(Entity&)> indiceCount;
		std::function<std::vector<uint32_t>(Entity&)> indices;
		std::function<uint32_t(Entity&)> vertexCount;
		std::function<std::vector<glm::vec3>(Entity&)> vertices;
		std::function<uint32_t(Entity&)> colorCount;
		std::function<std::vector<glm::vec4>(Entity&)> colors;
		std::function<uint32_t(Entity&)> uv2Count;
		std::function<std::vector<glm::vec2>(Entity&)> uv2s;
		std::function<uint32_t(Entity&)> uv3Count;
		std::function<std::vector<glm::vec3>(Entity&)> uv3s;
		std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> keyedTextures;
		shaders::RuntimeConstants constants;
		std::vector<size_t*> INDEX_STACK;
    };
}
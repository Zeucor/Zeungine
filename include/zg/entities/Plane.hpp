#pragma once
#include <array>
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/glm.hpp>

namespace zg
{
	struct Window;
}
namespace zg::entities
{
	EntityCreateInfo PlaneFactory(glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec2 size,
		glm::vec4 color, const shaders::RuntimeConstants& constants = {}, std::string_view name = "");
	EntityCreateInfo PlaneFactory(glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec2 size,
		textures::Texture& texture, const shaders::RuntimeConstants& constants = {}, std::string_view name = "");
	// struct Plane : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<Plane>::id; }
	// 	glm::vec4 color;
	// 	std::vector<glm::vec4> colors;
	// 	std::vector<glm::vec2> uvs;
	// 	std::vector<glm::vec3> normals = {};
	// 	textures::Texture* texturePointer = 0;
	// 	glm::vec2 size;
	// 	inline static size_t planesCount = 0;
	// 	Plane(Window& window, );
	// 	Plane(Window& window, );
	// 	std::vector<uint32_t> getIndices(zg::Window& window);
	// 	bool preRender() override;
	// 	void setColor(glm::vec4 color);
	// 	void setSize(glm::vec2 size);
	// };
} // namespace zg::entities

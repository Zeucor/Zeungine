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
	EntityCreateInfo FrameFactory(glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec2 size, float borderWidth,
		glm::vec4 color, const shaders::RuntimeConstants& constants = {}, std::string_view name = "");
	// struct Frame : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<Frame>::id; }
	// 	glm::vec4 color;
	// 	std::vector<glm::vec4> colors;
	// 	std::vector<glm::vec2> uvs;
	// 	std::vector<glm::vec3> normals = {};
	// 	textures::Texture* texturePointer = 0;
	// 	glm::vec2 size;
	// 	float borderWidth = 0.5f;
	// 	inline static size_t framesCount = 0;
	// 	Frame(Window& window, );
	// 	std::vector<uint32_t> getIndices(zg::Window& window);
	// 	std::vector<glm::vec3> getElements(glm::vec2 size, float borderWidth);
	// 	uint32_t getIndiceCount();
	// 	uint32_t getvertexCount();
	// 	bool preRender() override;
	// 	void setColor(glm::vec4 color);
	// 	void setSize(glm::vec2 size);
	// };
} // namespace zg::entities

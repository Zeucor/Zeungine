#pragma once
#include <zg/textures/Texture.hpp>
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <array>

namespace zg::entities
{
	EntityCreateInfo SkyBoxFactory(const std::vector<std::string_view> &texturePaths = {}, std::string_view name = "");
	// struct SkyBox : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<SkyBox>::id; }
	// 	std::vector<glm::vec3> uvs;
	// 	textures::Texture texture;
	// 	inline static size_t skyBoxesCount = 0;
	// 	explicit SkyBox(Window &window, );
	// 	bool preRender() override;
	// 	void postRender() override;
	// };
}
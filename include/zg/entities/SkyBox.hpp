#pragma once
#include <zg/textures/Texture.hpp>
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <array>
namespace zg::entities
{
	struct SkyBox : Entity
	{
    std::vector<glm::vec3> uvs;
    textures::Texture texture;
		Scene &scene;
		inline static size_t skyBoxesCount = 0;
    explicit SkyBox(Window &window, Scene &scene, const std::vector<std::string_view> &texturePaths = {}, std::string_view name = "");
		void preRender() override;
		void postRender() override;
  };
}
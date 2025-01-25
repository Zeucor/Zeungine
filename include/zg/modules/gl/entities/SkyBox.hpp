#pragma once
#include <zg/modules/gl/textures/Texture.hpp>
#include <zg/modules/gl/GLEntity.hpp>
#include <zg/modules/gl/GLScene.hpp>
#include <zg/modules/gl/GLWindow.hpp>
#include <array>
namespace zg::modules::gl::entities
{
	struct SkyBox : GLEntity
	{
    std::vector<glm::vec3> uvs;
    textures::Texture texture;
		GLScene &scene;
		inline static size_t skyBoxesCount = 0;
    explicit SkyBox(GLWindow &window, GLScene &scene, const std::vector<std::string_view> &texturePaths = {}, std::string_view name = "");
		void preRender() override;
		void postRender() override;
  };
}
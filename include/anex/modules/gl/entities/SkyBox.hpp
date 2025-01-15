#pragma once
#include <anex/modules/gl/textures/Texture.hpp>
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include <array>
namespace anex::modules::gl::entities
{
	struct SkyBox : GLEntity
	{
    std::vector<glm::vec3> uvs;
    textures::Texture texture;
		GLScene &scene;
    explicit SkyBox(GLWindow &window, GLScene &scene, const std::vector<std::string_view> &texturePaths = {});
		void preRender() override;
		void postRender() override;
  };
}
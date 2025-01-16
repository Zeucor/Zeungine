#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
#include <anex/modules/gl/entities/Plane.hpp>
#include <anex/modules/gl/entities/TextView.hpp>
#include <array>
#include <anex/modules/gl/entities/DropdownMenu.hpp>
namespace anex::modules::gl::entities
{
	struct Toolbar : GLEntity
	{
		size_t ID = 0;
		std::vector<glm::vec4> colors;
    GLScene &scene;
		fonts::freetype::FreetypeFont &font;
		std::string xString;
		std::shared_ptr<Plane> xButton;
		std::shared_ptr<TextView> xTextView;
		std::string maxString;
		std::shared_ptr<Plane> maxButton;
		std::shared_ptr<TextView> maxTextView;
		std::string _String;
		std::shared_ptr<Plane> _Button;
		std::shared_ptr<TextView> _TextView;
		std::shared_ptr<textures::Texture> iconTexture;
		std::shared_ptr<Plane> icon;
		std::string fileString;
		std::shared_ptr<Plane> file;
		std::shared_ptr<TextView> fileTextView;
		std::string editString;
		std::shared_ptr<Plane> edit;
		std::shared_ptr<TextView> editTextView;
		std::string helpString;
		std::shared_ptr<Plane> help;
		std::shared_ptr<TextView> helpTextView;
		IWindow::EventIdentifier xButtonLeftMousePressID = 0;
		IWindow::EventIdentifier xButtonMouseHoverID = 0;
		IWindow::EventIdentifier maxButtonLeftMousePressID = 0;
		IWindow::EventIdentifier maxButtonMouseHoverID = 0;
		IWindow::EventIdentifier _ButtonLeftMousePressID = 0;
		IWindow::EventIdentifier _ButtonMouseHoverID = 0;
		IWindow::EventIdentifier dragMousePressID = 0;
		IWindow::EventIdentifier globalDragMousePressID = 0;
		IWindow::EventIdentifier dragMouseMoveID = 0;
		bool dragEnabled = false;
		glm::vec2 dragOldCoords = glm::vec2(0, 0);
		float height = 0.0f;
		std::shared_ptr<DropdownMenu> fileDropdown;
		IWindow::EventIdentifier filePressID;
		size_t fileID = 0;
		size_t fileDropdownID = 0;
		IWindow::EventIdentifier fileHoverID = 0;
		std::shared_ptr<DropdownMenu> editDropdown;
		IWindow::EventIdentifier editPressID;
		size_t editID = 0;
		size_t editDropdownID = 0;
		IWindow::EventIdentifier editHoverID = 0;
		std::shared_ptr<DropdownMenu> helpDropdown;
		IWindow::EventIdentifier helpPressID;
		size_t helpID = 0;
		size_t helpDropdownID = 0;
		IWindow::EventIdentifier helpHoverID = 0;
  	Toolbar(GLWindow &window,
						GLScene &scene,
						const glm::vec3 &position,
						const glm::vec3 &rotation,
						const glm::vec3 &scale,
						const glm::vec4 &color,
						const float &height,
						fonts::freetype::FreetypeFont &font);
		~Toolbar();
    void preRender() override;
		void setSize(const glm::vec2 &newSize);
  };
}
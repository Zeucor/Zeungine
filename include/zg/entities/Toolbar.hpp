#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/entities/TextView.hpp>
#include <array>
#include <zg/entities/DropdownMenu.hpp>

namespace zg::entities
{
	EntityCreateInfo ToolbarFactory(
		glm::vec3 position,
		glm::quat rotation,
		glm::vec3 scale,
		glm::vec4 color,
		float height,
		fonts::freetype::FreetypeFont &font,
		std::string_view name = "");
	// struct Toolbar : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<Toolbar>::id; }
	// 	size_t ID = 0;
	// 	std::vector<glm::vec4> colors;
	// 	fonts::freetype::FreetypeFont &font;
	// 	Entity* xButton;
	// 	std::shared_ptr<textures::Texture> xButtonTexture;
	// 	Entity* xButtonImage;
	// 	Entity* maxButton;
	// 	std::shared_ptr<textures::Texture> maxButtonTexture;
	// 	Entity* maxButtonPlane;
	// 	Entity* minButton;
	// 	std::shared_ptr<textures::Texture> minButtonTexture;
	// 	Entity* minButtonPlane;
	// 	std::shared_ptr<textures::Texture> iconTexture;
	// 	Entity* icon;
	// 	std::string fileString;
	// 	Entity* file;
	// 	Entity* fileTextView;
	// 	std::string editString;
	// 	Entity* edit;
	// 	Entity* editTextView;
	// 	std::string helpString;
	// 	Entity* help;
	// 	Entity* helpTextView;
	// 	UniqueIdentifier xButtonLeftMousePressID = 0;
	// 	UniqueIdentifier xButtonMouseHoverID = 0;
	// 	UniqueIdentifier maxButtonLeftMousePressID = 0;
	// 	UniqueIdentifier maxButtonMouseHoverID = 0;
	// 	UniqueIdentifier minButtonLeftMousePressID = 0;
	// 	UniqueIdentifier minButtonMouseHoverID = 0;
	// 	UniqueIdentifier dragMousePressID = 0;
	// 	UniqueIdentifier globalDragMousePressID = 0;
	// 	UniqueIdentifier dragMouseMoveID = 0;
	// 	bool dragEnabled = false;
	// 	glm::vec2 dragOldCoords = glm::vec2(0, 0);
	// 	float height;
	// 	float NDCHeight;
	// 	Entity* fileDropdown;
	// 	UniqueIdentifier filePressID;
	// 	size_t fileID = 0;
	// 	size_t fileDropdownID = 0;
	// 	UniqueIdentifier fileHoverID = 0;
	// 	Entity* editDropdown;
	// 	UniqueIdentifier editPressID;
	// 	size_t editID = 0;
	// 	size_t editDropdownID = 0;
	// 	UniqueIdentifier editHoverID = 0;
	// 	Entity* helpDropdown;
	// 	UniqueIdentifier helpPressID;
	// 	size_t helpID = 0;
	// 	size_t helpDropdownID = 0;
	// 	UniqueIdentifier helpHoverID = 0;
	// 	inline static size_t toolbarsCount = 0;
	// 	Toolbar(Window &window,
	// 			Scene &scene,
	// 			glm::vec3 position,
	// 			glm::quat rotation,
	// 			glm::vec3 scale,
	// 			glm::vec4 color,
	// 			float height,
	// 			fonts::freetype::FreetypeFont &font,
	// 			std::string_view name = "");
	// 	~Toolbar();
	// 	bool preRender() override;
	// 	void setSize(glm::vec2 newSize);
	// };
}
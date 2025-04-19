#pragma once
#include <zg/textures/Texture.hpp>
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/interfaces/ISizable.hpp>
#include <array>

namespace zg::entities
{
	using RepositionHandler = std::function<glm::vec3(glm::vec2)>;
	using ResizeHandler = std::function<glm::vec2(glm::vec2)>;
	using ReFontSizeHandler = std::function<float()>;
	EntityCreateInfo TextViewFactory(
		glm::vec3 position,
		glm::quat rotation,
		glm::vec3 scale,
		glm::vec4 textColor,
		const std::string_view text,
		glm::vec2 size,
		fonts::freetype::FreetypeFont &font,
		float fontSize,
		bool textSizeIsNDC = true,
		glm::vec2 bounds = {0, 0},
		enums::EBreakStyle breakStyle = enums::EBreakStyle::Word,
		const RepositionHandler &repositionHandler = {},
		const ResizeHandler &resizeHandler = {},
		const ReFontSizeHandler &reFontSizeHandler = {},
		std::string_view name = "");
	// struct TextView : Entity, ISizable
	// {
	// 	size_t getTypeID() override { return EntityTypeID<TextView>::id; };
	// 	std::vector<glm::vec3> normals = {};
	// 	std::shared_ptr<textures::Texture> texturePointer;
	// 	glm::vec4 textColor;
	// 	std::string oldText;
	// 	std::string text;
	// 	glm::vec2 textSize;
	// 	glm::vec2 actualSizeBeforeNDC;
	// 	glm::vec2 actualSize;
	// 	fonts::freetype::FreetypeFont &font;
	// 	float fontSize;
	// 	bool textSizeIsNDC;
	// 	glm::vec2 bounds;
	// 	enums::EBreakStyle breakStyle;
	// 	RepositionHandler repositionHandler;
	// 	ResizeHandler resizeHandler;
	// 	ReFontSizeHandler reFontSizeHandler;
	// 	int64_t cursorIndex = 0;
	// 	glm::vec3 cursorPosition = glm::vec3(0);
	// 	UniqueIdentifier resizeID = 0;
	// 	std::vector<Entity*> existingAndUpdatedGlyphs;
	// 	Entity* cursor;
	// 	inline static size_t textViewsCount = 0;
	// 	explicit TextView(Window &window,
	// 					  );
	// 	~TextView() override;
	// 	void preUpdate() override;
	// 	void forceUpdate();
	// 	bool preRender() override;
	// 	void setSize(glm::vec3 size) override;
	// 	void updateText(const std::string_view text);
	// 	void setTextColor(glm::vec4 newTextColor);
	// 	void forceReposition();
	// };
}
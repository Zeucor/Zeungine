#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include "./Plane.hpp"
#include "./TextView.hpp"
#include <anex/glm.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>

namespace anex::modules::gl::entities
{
	struct Input : GLEntity
	{
		std::vector<glm::vec4> colors;
		GLScene& scene;
		glm::vec2 size;
		std::string text;
		std::shared_ptr<TextView> textView;
		fonts::freetype::FreetypeFont& font;
    float width;
		float height;
		float NDCWidth;
		float NDCHeight;
		std::string placeholderText;
		std::shared_ptr<TextView> placeholderTextView;
		std::shared_ptr<TextView> activeTextView;
		float fontSize;
		GLWindow::EventIdentifier mouseHoverID = 0;
		GLWindow::EventIdentifier mousePressID = 0;
		GLWindow::EventIdentifier anyKeyPressID = 0;
		bool active = false;
		bool hovered = false;
		glm::vec4 activeColor;
		glm::vec4 inactiveColor;
		std::shared_ptr<Plane> cursorPlane;
		float padding;
		float NDCPadding;
		inline static size_t inputsCount = 0;
		inline static Input *activeInput = 0;
		Input(GLWindow& window,
					GLScene& scene,
					glm::vec3 position,
					glm::vec3 rotation,
					glm::vec3 scale,
					glm::vec4 backgroundColor,
					fonts::freetype::FreetypeFont& font,
          float width,
					float height,
          const std::string& placeholderText,
          float padding = 8,
					const shaders::RuntimeConstants& constants = {},
					const std::string_view name = "");
		~Input();
		void preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec2  size);
		void showTextView(const std::shared_ptr<TextView>& showTextView);
		char getShiftedChar(const char &key, bool shiftPressed);
	};
}

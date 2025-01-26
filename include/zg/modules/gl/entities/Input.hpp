#pragma once
#include <zg/modules/gl/GLEntity.hpp>
#include <zg/modules/gl/GLScene.hpp>
#include <zg/modules/gl/RenderWindow.hpp>
#include "./Plane.hpp"
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/modules/gl/fonts/freetype/Freetype.hpp>

namespace zg::modules::gl::entities
{
	struct Input : GLEntity
	{
		std::vector<glm::vec4> colors;
		GLScene& scene;
		glm::vec2 size;
		std::string *textPointer = 0;
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
		RenderWindow::EventIdentifier mouseHoverID = 0;
		RenderWindow::EventIdentifier mousePressID = 0;
		RenderWindow::EventIdentifier anyKeyPressID = 0;
		bool active = false;
		bool hovered = false;
		glm::vec4 activeColor;
		glm::vec4 inactiveColor;
		std::shared_ptr<Plane> cursorPlane;
		float padding;
		float NDCPadding;
		inline static size_t inputsCount = 0;
		inline static Input *activeInput = 0;
		Input(RenderWindow& window,
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
		void setActive();
		void setInactive();
		void clear();
		void handleKey(IWindow::Key key, bool pressed);
	};
}

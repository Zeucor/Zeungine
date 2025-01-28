#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include "./Plane.hpp"
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/fonts/freetype/Freetype.hpp>

namespace zg::entities
{
	struct Input : Entity
	{
		std::vector<glm::vec4> colors;
		Scene& scene;
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
		Window::EventIdentifier mouseHoverID = 0;
		Window::EventIdentifier mousePressID = 0;
		Window::EventIdentifier anyKeyPressID = 0;
		bool active = false;
		bool hovered = false;
		glm::vec4 activeColor;
		glm::vec4 inactiveColor;
		std::shared_ptr<Plane> cursorPlane;
		float padding;
		float NDCPadding;
		inline static size_t inputsCount = 0;
		inline static Input *activeInput = 0;
		Input(Window& window,
					Scene& scene,
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

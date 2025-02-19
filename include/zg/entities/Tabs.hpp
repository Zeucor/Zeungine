#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/zgfilesystem/File.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/glm.hpp>
#include "./TextView.hpp"
#include "./Plane.hpp"
namespace zg::entities
{
	struct Tab;
	struct TabsBar : Entity
	{
		std::vector<glm::vec4> colors;
		Scene& scene;
		glm::vec2 size;
		glm::vec4 color;
		fonts::freetype::FreetypeFont& font;
		float width;
		float height;
		std::shared_ptr<TextView> titleTextView;
		inline static size_t tabBarsCount = 0;
		using TabClickHandler = std::function<void()>;
		TabsBar(Window& window, Scene& scene, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec4 color,
						fonts::freetype::FreetypeFont& font, float width, float height,
						const shaders::RuntimeConstants& constants = {}, std::string_view name = "");
		size_t addTab(std::string_view name, const TabClickHandler& handler, bool active = false,
									const zgfilesystem::File& iconFile = {});
		void removeTab(size_t ID);
		bool preRender() override;
		void setColor(glm::vec4 color);
		void setSize();
		void markInactive(Tab* activeTab);
	};

	struct Tab : Entity
	{
		std::vector<glm::vec4> colors;
		Scene& scene;
		glm::vec2 size;
		std::string text;
		std::shared_ptr<TextView> textView;
		fonts::freetype::FreetypeFont& font;
		float height;
		float NDCHeight;
		TabsBar::TabClickHandler handler;
		EventIdentifier mouseHoverID = 0;
		EventIdentifier mousePressID = 0;
		bool active;
		bool hovered = false;
		glm::vec4 activeColor;
		glm::vec4 inactiveColor;
		TabsBar& tabsBar;
		std::shared_ptr<textures::Texture> iconTexture;
		std::shared_ptr<entities::Plane> iconPlane;
		inline static size_t tabsCount = 0;
		Tab(Window& window, Scene& scene, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec4 color,
				std::string_view text, fonts::freetype::FreetypeFont& font, float height,
				const TabsBar::TabClickHandler& handler, bool active, TabsBar& tabsBar, const zgfilesystem::File& iconFile = {},
				const shaders::RuntimeConstants& constants = {}, std::string_view name = "");
		~Tab() override;
		bool preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec2 size);
		void markInactive();
	};
} // namespace zg::entities

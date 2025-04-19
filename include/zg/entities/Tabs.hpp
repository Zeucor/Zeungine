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
	using TabClickHandler = std::function<void()>;
	EntityCreateInfo TabsBarFactory(glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec4 color,
		fonts::freetype::FreetypeFont& font, float width, float height,
		const shaders::RuntimeConstants& constants = {}, std::string_view name = "");
	// struct TabsBar : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<TabsBar>::id; }
	// 	std::vector<glm::vec4> colors;
	// 	glm::vec2 size;
	// 	glm::vec4 color;
	// 	fonts::freetype::FreetypeFont& font;
	// 	float width;
	// 	float height;
	// 	Entity* titleTextView;
	// 	inline static size_t tabBarsCount = 0;
	// 	TabsBar(Window& window, );
	// 	size_t addTab(std::string_view name, const TabClickHandler& handler, bool active = false,
	// 								const zgfilesystem::File& iconFile = {});
	// 	void removeTab(size_t ID);
	// 	bool preRender() override;
	// 	void setColor(glm::vec4 color);
	// 	void setSize();
	// 	void markInactive(Tab* activeTab);
	// };

	EntityCreateInfo TabFactory(glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec4 color,
		std::string_view text, fonts::freetype::FreetypeFont& font, float height,
		const TabClickHandler& handler, bool active, Entity& tabsBar, const zgfilesystem::File& iconFile = {},
		const shaders::RuntimeConstants& constants = {}, std::string_view name = "");
	// struct Tab : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<Tab>::id; }
	// 	std::vector<glm::vec4> colors;
	// 	glm::vec2 size;
	// 	std::string text;
	// 	Entity* textView;
	// 	fonts::freetype::FreetypeFont& font;
	// 	float height;
	// 	float NDCHeight;
	// 	TabsBar::TabClickHandler handler;
	// 	UniqueIdentifier mouseHoverID = 0;
	// 	UniqueIdentifier mousePressID = 0;
	// 	bool active;
	// 	bool hovered = false;
	// 	glm::vec4 activeColor;
	// 	glm::vec4 inactiveColor;
	// 	TabsBar& tabsBar;
	// 	std::shared_ptr<textures::Texture> iconTexture;
	// 	Entity* iconPlane;
	// 	inline static size_t tabsCount = 0;
	// 	Tab(Window& window, );
	// 	~Tab() override;
	// 	bool preRender() override;
	// 	void setColor(glm::vec4 color);
	// 	void setSize(glm::vec2 size);
	// 	void markInactive();
	// };
} // namespace zg::entities

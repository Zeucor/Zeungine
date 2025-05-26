#include <zg/entities/StatusText.hpp>
#include <zg/utilities.hpp>
using namespace zg::entities;
// StatusText::StatusText(zg::Window &window,
// 					   zg::Scene &scene,
// 					   glm::vec3 position,
// 					   glm::quat rotation,
// 					   glm::vec3 scale,
// 					   glm::vec4 color,
// 					   fonts::freetype::FreetypeFont &font,
// 					   float width,
// 					   float height,
// 					   std::string_view text,
// 					   const zg::shaders::RuntimeConstants &constants,
// 					   std::string_view name) : zg::Entity(window, scene,
// 														   zg::mergeVectors<std::string>({{"Color", "Position",
// 																								"View", "Projection", "Model", "CameraPosition"}},
// 																							  constants),
// 														   6,
// 														   {0, 1, 2, 2, 3, 0},
// 														   4,
// 														   {{0, -0, 0}, {0, -0, 0}, {0, 0, 0}, {0, 0, 0}},
// 														   position,
// 														   rotation,
// 														   scale,
// 														   name.empty() ? "StatusText " + std::to_string(++statusTextsCount) : name),
												
// 												size({0, 0}),
// 												color(color),
// 												font(font),
// 												width(width),
// 												height(height),
// 												NDCHeight((height / window.windowHeight) * 2)
// {
// 	updateIndices(indices);
// 	setColor(color);
// 	setSize();
// 	float FontSize = height / 1.5f;
// 	float LineHeight = 0;
// 	auto TextSize = font.stringSize(text, FontSize, LineHeight, {0, 0});
// 	TextSize.y /= window.windowHeight * 0.5f;
// 	TextSize.x /= window.windowWidth * 0.5f;
// 	textView = std::make_shared<TextView>(
// 		window,
// 		scene,
// 		glm::vec3(0),
// 		glm::vec3(0),
// 		glm::vec3(1),
// 		glm::vec4(1, 1, 1, 1),
// 		text,
// 		TextSize,
// 		font,
// 		FontSize,
// 		true,
// 		glm::vec2(0, 0),
// 		enums::EBreakStyle::None,
// 		[&](auto textSize)
// 		{
// 			return glm::vec3(textSize.x / 2 + 8 / this->window.windowWidth / 0.5, -NDCHeight / 2, 0.1);
// 		},
// 		TextView::ResizeHandler(),
// 		[&]
// 		{
// 			return NDCHeight * this->window.windowHeight * 0.5f / 1.5f;
// 		});
// 	textView->addToBVH = false;
// 	addEntity(textView);
// 	addToBVH = false;
// }
// void StatusText::setColor(glm::vec4 newColor)
// {
// 	colors = {newColor, newColor, newColor, newColor};
// 	updateElements("Color", colors);
// };
// void StatusText::setSize()
// {
// 	glm::vec2 newSize(width / this->window.windowWidth / 0.5, height / this->window.windowHeight / 0.5);
// 	vertices = {
// 		{0, -newSize.y, 0}, {newSize.x, -newSize.y, 0}, {newSize.x, 0, 0}, {0, 0, 0}};
// 	updateElements("Position", vertices);
// 	this->size = newSize;
// }
// void StatusText::setText(std::string_view text)
// {
// 	textView->text = text;
// }
// void StatusText::setTextColor(glm::vec4 newTextColor)
// {
// 	textView->setTextColor(newTextColor);
// }
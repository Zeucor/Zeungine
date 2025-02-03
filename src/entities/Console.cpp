#include <zg/entities/Console.hpp>
#include <zg/utilities.hpp>
#include <iostream>
using namespace zg::entities;
Console::Console(zg::Window &window,
				             zg::Scene &scene,
				             glm::vec3 position,
				             glm::vec3 rotation,
				             glm::vec3 scale,
				             glm::vec4 backgroundColor,
				             fonts::freetype::FreetypeFont &font,
										 float width,
										 float height,
				             const zg::shaders::RuntimeConstants &constants,
                     std::string_view name):
	zg::Entity(
		window,
		zg::mergeVectors<std::string_view>({
			{
				"Color", "Position",
				"View", "Projection", "Model", "CameraPosition"
			}
		}, constants),
		6,
		{0, 1, 2,  2, 3, 0},
    4,
		{
			{ 0, -0, 0 }, { 0, -0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }
		},
		position,
		rotation,
		scale,
		name.empty() ? "Console " + std::to_string(++consolesCount) : name
	),
	scene(scene),
	backgroundColor(backgroundColor),
	font(font),
	width(width),
	height(height),
	hookedConsole([&](auto &lines) {
		hookedCallback(lines);
	})
{
	updateIndices(indices);
	setBackgroundColor(backgroundColor);
	updateElements("Position", positions);
	Console::setSize(glm::vec3(0));
	this->addMousePressHandler(3, [&](auto pressed)
	{
		if (pressed && currentIndex < consoleTextViews.size() - 1)
		{
			currentIndex++;
			showConsoleLines();
		}
	});
	this->addMousePressHandler(4, [&](auto pressed)
	{
		if (pressed && currentIndex > 0)
		{
			currentIndex--;
			showConsoleLines();
		}
	});
};
void Console::preRender()
{
	const auto &model = getModelMatrix();
	shader.bind();
	scene.entityPreRender(*this);
	shader.setBlock("Model", *this, model);
	shader.setBlock("View", *this, scene.view.matrix);
	shader.setBlock("Projection", *this, scene.projection.matrix);
	shader.setBlock("CameraPosition", *this, scene.view.position, 16);
	shader.unbind();
};
void Console::setBackgroundColor(glm::vec4 newBackgroundColor)
{
	backgroundColor = newBackgroundColor;
	colors = {backgroundColor, backgroundColor, backgroundColor, backgroundColor};
	updateElements("Color", colors);
}
void Console::setSize(glm::vec3 newSize)
{
	glm::vec3 actualNewSize(width / window.windowWidth / 0.5, height / window.windowHeight / 0.5, 0);
	positions = {
		{ 0, -actualNewSize.y, 0 }, { actualNewSize.x, -actualNewSize.y, 0 }, { actualNewSize.x, 0, 0 }, { 0, 0, 0 }
	};
	updateElements("Position", positions);
	size = actualNewSize;
}
void Console::hookedCallback(const std::vector<std::string> &lines)
{
	window.runOnThread([&, lines](auto &iwindow)
	{
		auto addLine = [&](auto& line)
		{
			auto index = consoleTextViews.size();
			consoleTextViews.resize(index + 1);
			static auto indent = 8 / window.windowWidth / 0.5;
			auto consoleTextView = std::make_shared<TextView>(
				window,
				scene,
				glm::vec3(0),
				glm::vec3(0),
				glm::vec3(1),
				glm::vec4(1, 1, 1, 1),
				line,
				glm::vec2(0),
				font,
				0.f,
				true,
				[&, index](auto titleSize)
				{
					auto offsetIndex = consoleTextViews.size() - index - 1 - currentIndex;
					auto offset = (offsetIndex * titleSize.y);
					return glm::vec3(titleSize.x / 2 + indent, -size.y + titleSize.y / 2 + offset, 0.1);
				},
				TextView::ResizeHandler(),
				[&]
				{
					return this->window.windowHeight / 46.f;
				});
			consoleTextViews[index] = consoleTextView;
			consoleTextView->addToBVH = false;
		};
		for (const auto &line : lines)
		{
			addLine(line);
		}
		showConsoleLines();
	});
}
void Console::showConsoleLines()
{
	if (consoleTextViews.empty())
		return;
	size_t count = 0;
	for (auto revIt = consoleTextViews.rbegin(); revIt != consoleTextViews.rend(); revIt++)
	{
		if (!(*revIt)->text.empty())
		{
			count = size_t(size.y / (*revIt)->size.y);
			break;
		}
	}
	auto start = consoleTextViews.rbegin() + currentIndex;
	auto end = consoleTextViews.rend();
	if (currentIndex + count <= consoleTextViews.size())
	{
		end = start + count;
	}
	for (auto it = start; it != end; ++it)
	{
		auto &entity = *it;
		if (!entity->ID)
			addChild(entity);
		entity->forceReposition();
	}
	auto removeStart = end;
	auto removeEnd = consoleTextViews.rend();
	for (auto it = removeStart; it != removeEnd; ++it)
	{
		auto &entity = *it;
		if (entity->ID)
			removeChild(entity->ID);
	}
	if (start != consoleTextViews.rbegin())
	{
		removeStart = start - 1;
		removeEnd = consoleTextViews.rbegin();
		for (auto it = removeStart; it != removeEnd; --it)
		{
			auto &entity = *it;
			if (entity->ID)
				removeChild(entity->ID);
		}
		auto &entity = *removeEnd;
		if (entity->ID)
			removeChild(entity->ID);
	}
}
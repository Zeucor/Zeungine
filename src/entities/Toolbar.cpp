#include <zg/entities/Toolbar.hpp>
#include <zg/images/SVGRasterize.hpp>
#include <iostream>
using namespace zg::entities;
Toolbar::Toolbar(Window &_window,
				 Scene &scene,
				 glm::vec3 position,
				 glm::vec3 rotation,
				 glm::vec3 scale,
				 glm::vec4 color,
				 float height,
				 fonts::freetype::FreetypeFont &font,
				 std::string_view name) : Entity(_window, {"Color", "Position", "View", "Projection", "Model", "CameraPosition"}, 6,
												 {
													 0, 1, 2, 2, 3, 0 // Front face
												 },
												 4,
												 {
													 {0, (-height / _window.windowHeight) * 2, 0}, {2, (-height / _window.windowHeight) * 2, 0}, {2, 0, 0}, {0, 0, 0} // Front
												 },
												 position, rotation, scale,
												 name.empty() ? "Toolbar " + std::to_string(++toolbarsCount) : name),
										  colors({color, color, color, color}),
										  scene(scene),
										  font(font),
										  height(height),
										  NDCHeight((height / _window.windowHeight) * 2)
{
	updateIndices(indices);
	updateElements("Color", colors);
	updateElements("Position", positions);
	//
	float toolButtonsX = 2;
	float svgScale = 1;
	auto executableDirectory = zgfilesystem::File::getProgramDirectoryPath();
	float imageSize = height * svgScale;
	float imageSizePlane = imageSize / svgScale;
	// Close Button
	xButton = std::make_shared<Plane>(window, scene, glm::vec3(toolButtonsX = (toolButtonsX - (NDCHeight / 4)), -NDCHeight / 2, 0.1), glm::vec3(0), glm::vec3(1), glm::vec2(NDCHeight / 2.f, NDCHeight), glm::vec4(1, 0, 0, 1));
	addChild(xButton);
	auto closeCircleLinePath = (std::filesystem::path(executableDirectory) / "icons" / "Remix" / "System" / "close-circle-line.svg").string();
	auto closeCircleBitmap = images::SVGRasterize({closeCircleLinePath, enums::EFileLocation::Absolute, "r"}, {imageSize, imageSize});
	xButtonTexture = std::make_shared<textures::Texture>(
		window,
		glm::vec4(imageSize, imageSize, 1, 1),
		(void*)closeCircleBitmap.get(),
		textures::Texture::Format::RGBA8,
		textures::Texture::Type::UnsignedByte,
		textures::Texture::FilterType::Linear
	);
	xButtonImage = std::make_shared<entities::Plane>(
		window,
		scene,
		glm::vec3(0, 0, 0.1),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec2(imageSizePlane / window.windowWidth, imageSizePlane / window.windowHeight),
		*xButtonTexture
	);
	xButton->addChild(xButtonImage);
	xButtonImage->addToBVH = false;
	xButtonLeftMousePressID = xButton->addMousePressHandler(0, [&](auto pressed)
															{ window.close(); });
	xButtonMouseHoverID = xButton->addMouseHoverHandler([&](auto hovered)
														{
		if (hovered)
		{
			xButton->setColor({0.8, 0, 0, 1});
		}
		else
		{
			xButton->setColor({1, 0, 0, 1});
		} });
	// max Button
	maxButton = std::make_shared<Plane>(window, scene, glm::vec3(toolButtonsX = (toolButtonsX - (xButton->size.x)), -NDCHeight / 2, 0.1), glm::vec3(0), glm::vec3(1), glm::vec2(NDCHeight / 2.f, NDCHeight), glm::vec4(0.4f, 0.4f, 0.4f, 1));
	addChild(maxButton);
	auto fullscreenLinePath = (std::filesystem::path(executableDirectory) / "icons" / "Remix" / "Media" / "fullscreen-line.svg").string();
	auto fullscreenBitmap = images::SVGRasterize({fullscreenLinePath, enums::EFileLocation::Absolute, "r"}, {imageSize, imageSize});
	maxButtonTexture = std::make_shared<textures::Texture>(
		window,
		glm::vec4(imageSize, imageSize, 1, 1),
		(void*)fullscreenBitmap.get(),
		textures::Texture::Format::RGBA8,
		textures::Texture::Type::UnsignedByte,
		textures::Texture::FilterType::Linear
	);
	maxButtonPlane = std::make_shared<entities::Plane>(
		window,
		scene,
		glm::vec3(0, 0, 0.1),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec2(imageSizePlane / window.windowWidth, imageSizePlane / window.windowHeight),
		*maxButtonTexture
	);
	maxButton->addChild(maxButtonPlane);
	maxButtonPlane->addToBVH = false;
	maxButtonLeftMousePressID = maxButton->addMousePressHandler(0, [&](auto pressed)
																{ window.maximize(); });
	maxButtonMouseHoverID = maxButton->addMouseHoverHandler([&](auto hovered)
															{
		if (hovered)
		{
			maxButton->setColor({0.2, 0.2, 0.2, 1});
		}
		else
		{
			maxButton->setColor({0.4, 0.4, 0.4, 1});
		} });
	// _ Button
	minButton = std::make_shared<Plane>(window, scene, glm::vec3(toolButtonsX = (toolButtonsX - (maxButton->size.x)), -NDCHeight / 2, 0.1), glm::vec3(0), glm::vec3(1), glm::vec2(NDCHeight / 2.f, NDCHeight), glm::vec4(0.4f, 0.4f, 0.4f, 1));
	addChild(minButton);
	auto skipDownLinePath = (std::filesystem::path(executableDirectory) / "icons" / "Remix" / "Arrows" / "skip-down-line.svg").string();
	auto skipDownBitmap = images::SVGRasterize({skipDownLinePath, enums::EFileLocation::Absolute, "r"}, {imageSize, imageSize});
	minButtonTexture = std::make_shared<textures::Texture>(
		window,
		glm::vec4(imageSize, imageSize, 1, 1),
		(void*)skipDownBitmap.get(),
		textures::Texture::Format::RGBA8,
		textures::Texture::Type::UnsignedByte,
		textures::Texture::FilterType::Linear
	);
	minButtonPlane = std::make_shared<entities::Plane>(
		window,
		scene,
		glm::vec3(0, 0, 0.1),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec2(imageSizePlane / window.windowWidth, imageSizePlane / window.windowHeight),
		*minButtonTexture
	);
	minButton->addChild(minButtonPlane);
	minButtonPlane->addToBVH = false;
	minButtonLeftMousePressID = minButton->addMousePressHandler(0, [&](auto pressed)
															{ window.minimize(); });
	minButtonMouseHoverID = minButton->addMouseHoverHandler([&](auto hovered)
														{
		if (hovered)
		{
			minButton->setColor({0.2, 0.2, 0.2, 1});
		}
		else
		{
			minButton->setColor({0.4, 0.4, 0.4, 1});
		} });
	// icon
	iconTexture.reset(new textures::Texture(window, {128, 128, 1, 0}, std::string_view("images/zeungine-icon.png")));
	icon = std::make_shared<Plane>(window, scene, glm::vec3(NDCHeight / 4, -NDCHeight / 2, 0.1), glm::vec3(0), glm::vec3(1), glm::vec2(NDCHeight / 2, NDCHeight), *iconTexture);
	addChild(icon);
	//
	float toolOptionsX = NDCHeight / 1.5;
	float padding = NDCHeight / 10;
	// File
	fileString = "File";
	float fileFontSize = (height / 2.f);
	float fileLineHeight = 0;
	auto fileTextSize = font.stringSize(fileString, fileFontSize, fileLineHeight, {0, 0});
	fileTextSize.x /= window.windowWidth * 0.5f;
	fileTextSize.y /= window.windowHeight * 0.5f;
	file = std::make_shared<Plane>(
		window,
		scene,
		glm::vec3(toolOptionsX = (toolOptionsX + fileTextSize.x / 4 + padding), -fileTextSize.y / 2, 0.1),
		glm::vec3(0),
		glm::vec3(1),
		fileTextSize,
		color);
	fileID = addChild(file);
	fileTextView = std::make_shared<TextView>(
		window,
		scene,
		glm::vec3(0, 0, 0.1f),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec4(1, 1, 1, 1),
		fileString,
		fileTextSize / 1.5f,
		font,
		fileFontSize,
		true,
		glm::vec2(0, 0),
		enums::EBreakStyle::None,
		TextView::RepositionHandler(),
		TextView::ResizeHandler(),
		[&]
		{
			return NDCHeight * window.windowHeight * 0.5f / 2.f;
		});
	fileTextView->addToBVH = false;
	file->addChild(fileTextView);
	fileDropdown = std::make_shared<DropdownMenu>(window, scene, glm::vec3(-file->size.x / 2, -file->size.y / 2, 0.1), glm::vec3(0), glm::vec3(1), glm::vec4(0, 0, 0, 1), font);
	filePressID = file->addMousePressHandler(0, [&](auto pressed)
											 {
		if (!pressed)
		{
			if (fileDropdownID)
			{
				scene.bvh->removeEntity(scene, *fileDropdown);
				file->removeChild(fileDropdownID);
			}
			else
			{
				fileDropdownID = file->addChild(fileDropdown);
				std::vector<size_t> entityIDs({ID, fileID, fileDropdownID});
				scene.postAddEntity(fileDropdown, entityIDs);
				if (editDropdownID)
					edit->callMousePressHandler(0, false);
				if (helpDropdownID)
					help->callMousePressHandler(0, false);
			}
		} });
	fileHoverID = file->addMouseHoverHandler([&, color](auto hovered)
											 {
		if (hovered)
		{
			file->setColor({0.6, 0.6, 0.6, 1});
			if (editDropdownID || helpDropdownID)
			{
				file->callMousePressHandler(0, false);
			}
		}
		else
		{
			file->setColor(color);
		} });
	// Edit
	editString = "Edit";
	float editFontSize = height / 2;
	float editLineHeight = 0;
	auto editTextSize = font.stringSize(editString, editFontSize, editLineHeight, {0, 0});
	editTextSize.y /= window.windowHeight * 0.5f;
	editTextSize.x /= window.windowWidth * 0.5f;
	edit = std::make_shared<Plane>(
		window,
		scene,
		glm::vec3(toolOptionsX = (toolOptionsX + fileTextSize.x / 2 + editTextSize.x / 2 + padding), -editTextSize.y / 2, 0.1),
		glm::vec3(0),
		glm::vec3(1),
		editTextSize,
		color);
	editID = addChild(edit);
	editTextView = std::make_shared<TextView>(
		window,
		scene,
		glm::vec3(0, 0, 0.1f),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec4(1, 1, 1, 1),
		editString, editTextSize / 1.5f,
		font,
		editFontSize,
		true,
		glm::vec2(0, 0),
		enums::EBreakStyle::None,
		TextView::RepositionHandler(),
		TextView::ResizeHandler(),
		[&]
		{
			return NDCHeight * window.windowHeight * 0.5f / 2.f;
		});
	editTextView->addToBVH = false;
	edit->addChild(editTextView);
	editDropdown = std::make_shared<DropdownMenu>(window, scene, glm::vec3(-edit->size.x / 2, -edit->size.y / 2, 0.1), glm::vec3(0), glm::vec3(1), glm::vec4(0, 0, 0, 1), font);
	editDropdown->addOption("Undo", []() {

	});
	editDropdown->addOption("Redo", []() {

	});
	editDropdown->addOption("Cut", []() {

	});
	editDropdown->addOption("Copy", []() {

	});
	editDropdown->addOption("Paste", []() {

	});
	editDropdown->addOption("Delete", []() {

	});
	editDropdown->addOption("Duplicate", []() {

	});
	editDropdown->addOption("Find/Replace", []() {

	});
	editDropdown->addOption("Preferences", []() {

	});
	editPressID = edit->addMousePressHandler(0, [&](auto pressed)
											 {
		if (!pressed)
		{
			if (editDropdownID)
			{
				scene.bvh->removeEntity(scene, *editDropdown);
				edit->removeChild(editDropdownID);
			}
			else
			{
				editDropdownID = edit->addChild(editDropdown);
				std::vector<size_t> entityIDs({ID, editID, editDropdownID});
				scene.postAddEntity(editDropdown, entityIDs);
				if (fileDropdownID)
					file->callMousePressHandler(0, false);
				if (helpDropdownID)
					help->callMousePressHandler(0, false);
			}
		} });
	editHoverID = edit->addMouseHoverHandler([&, color](auto hovered)
											 {
		if (hovered)
		{
			edit->setColor({0.6, 0.6, 0.6, 1});
			if (fileDropdownID || helpDropdownID)
			{
				edit->callMousePressHandler(0, false);
			}
		}
		else
		{
			edit->setColor(color);
		} });
	// Help
	helpString = "Help";
	float helpFontSize = height / 2;
	float helpLineHeight = 0;
	auto helpTextSize = font.stringSize(helpString, helpFontSize, helpLineHeight, {0, 0});
	helpTextSize.y /= window.windowHeight * 0.5f;
	helpTextSize.x /= window.windowWidth * 0.5f;
	help = std::make_shared<Plane>(
		window,
		scene,
		glm::vec3(toolOptionsX = (toolOptionsX + helpTextSize.x / 2 + helpTextSize.x / 2 + padding / 2), -helpTextSize.y / 2, 0.1),
		glm::vec3(0),
		glm::vec3(1),
		helpTextSize,
		color);
	helpID = addChild(help);
	helpTextView = std::make_shared<TextView>(
		window,
		scene,
		glm::vec3(0, 0, 0.1f),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec4(1, 1, 1, 1),
		helpString,
		helpTextSize / 1.5f,
		font,
		helpFontSize,
		true,
		glm::vec2(0, 0),
		enums::EBreakStyle::None,
		TextView::RepositionHandler(),
		TextView::ResizeHandler(),
		[&]
		{
			return NDCHeight * window.windowHeight * 0.5f / 2.f;
		});
	helpTextView->addToBVH = false;
	help->addChild(helpTextView);
	helpDropdown = std::make_shared<DropdownMenu>(window, scene, glm::vec3(-help->size.x / 2, -help->size.y / 2, 0.1), glm::vec3(0), glm::vec3(1), glm::vec4(0, 0, 0, 1), font);
	helpDropdown->addOption("Documentation", []() {

	});
	helpDropdown->addOption("Tutorials", []() {

	});
	helpDropdown->addOption("Keyboard Shortcuts", []() {

	});
	helpDropdown->addOption("Report Bug", []() {

	});
	helpDropdown->addOption("About", []() {

	});
	helpPressID = help->addMousePressHandler(0, [&](auto pressed)
											 {
		if (!pressed)
		{
			if (helpDropdownID)
			{
				scene.bvh->removeEntity(scene, *helpDropdown);
				help->removeChild(helpDropdownID);
			}
			else
			{
				helpDropdownID = help->addChild(helpDropdown);
				std::vector<size_t> entityIDs({ID, helpID, helpDropdownID});
				scene.postAddEntity(helpDropdown, entityIDs);
				if (fileDropdownID)
					file->callMousePressHandler(0, false);
				if (editDropdownID)
					edit->callMousePressHandler(0, false);
			}
		} });
	helpHoverID = help->addMouseHoverHandler([&, color](auto hovered)
											 {
		if (hovered)
		{
			help->setColor({0.6, 0.6, 0.6, 1});
			if (editDropdownID || fileDropdownID)
			{
				help->callMousePressHandler(0, false);
			}
		}
		else
		{
			help->setColor(color);
		} });
	// Toolbar dragging
	dragMousePressID = addMousePressHandler(0, [&](auto pressed)
											{
		if (pressed && !dragEnabled)
		{
			window.mouseCapture(true);
		}
		else if (!pressed && dragEnabled)
		{
			dragOldCoords = {0, 0};
			window.mouseCapture(false);
		}
		dragEnabled = pressed; });
	globalDragMousePressID = window.addMousePressHandler(0, [&](auto pressed)
														 {
		if (!pressed && dragEnabled)
		{
			dragEnabled = pressed;
			dragOldCoords = {0, 0};
			window.mouseCapture(false);
		} });
	dragMouseMoveID = window.addMouseMoveHandler([&](auto coords)
												 {
		if (!dragEnabled)
		{
			return;
		}
		if (dragOldCoords.x == 0 && dragOldCoords.y == 0)
		{
			dragOldCoords = coords;
			return;
		}
		auto diff = coords - dragOldCoords;
		window.setXY(window.windowX + diff.x, window.windowY - diff.y);
		dragOldCoords.x = coords.x - diff.x;
		dragOldCoords.y = coords.y - diff.y; });
};
Toolbar::~Toolbar()
{
	xButton->removeMousePressHandler(0, xButtonLeftMousePressID);
	xButton->removeMouseHoverHandler(xButtonMouseHoverID);
	maxButton->removeMousePressHandler(0, maxButtonLeftMousePressID);
	maxButton->removeMouseHoverHandler(maxButtonMouseHoverID);
	minButton->removeMousePressHandler(0, minButtonLeftMousePressID);
	minButton->removeMouseHoverHandler(minButtonMouseHoverID);
	removeMousePressHandler(0, dragMousePressID);
	window.removeMousePressHandler(0, globalDragMousePressID);
	removeMouseMoveHandler(dragMouseMoveID);
	file->removeMousePressHandler(0, filePressID);
	file->removeMouseHoverHandler(fileHoverID);
	edit->removeMousePressHandler(0, editPressID);
	edit->removeMouseHoverHandler(editHoverID);
	help->removeMousePressHandler(0, helpPressID);
	help->removeMouseHoverHandler(helpHoverID);
}
bool Toolbar::preRender()
{
	const auto &model = getModelMatrix();
	shader->bind(*this);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, scene.view.matrix);
	shader->setBlock("Projection", *this, scene.projection.matrix);
	shader->setBlock("CameraPosition", *this, scene.view.position, 16);
	shader->unbind();
	return true;
};
void Toolbar::setSize(glm::vec2 newSize) {
	// NDCHeight = newSize.y;
	// positions = {
	// 	{ 0, -NDCHeight, 0}, { newSize.x, -NDCHeight, 0}, { newSize.x, 0, 0 }, { 0, 0, 0} // Front
	// };
	// updateElements("Position", positions);
	// xButton->setSize({ NDCHeight, NDCHeight });
	// maxButton->setSize({ NDCHeight, NDCHeight });
	// _Button->setSize({ NDCHeight, NDCHeight });
	// float toolButtonsX = newSize.x, toolButtonsY = -NDCHeight / 2;
	// xButton->position = { toolButtonsX = (toolButtonsX - (NDCHeight / 2)), toolButtonsY, 0.5};
	// maxButton->position = { toolButtonsX = (toolButtonsX - xButton->size.x), toolButtonsY, 0.5};
	// _Button->position = { toolButtonsX = (toolButtonsX - maxButton->size.x), toolButtonsY, 0.5};
	// float xFontSize = height;
	// float xLineHeight = 0;
	// auto xTextSize = font.stringSize(xString, xFontSize, xLineHeight, {0, 0});
	// xTextView->setSize(xTextSize / 2.f);
	// float maxFontSize = height;
	// float maxLineHeight = 0;
	// auto maxTextSize = font.stringSize(maxString, maxFontSize, maxLineHeight, {0, 0});
	// maxTextView->setSize(maxTextSize / 2.f);
	// float FontSize_ = height;
	// float LineHeight_ = 0;
	// auto TextSize_ = font.stringSize(_String, FontSize_, LineHeight_, {0, 0});
	// _TextView->setSize(TextSize_ / 2.f);
	// icon->position = {height / 2, -height / 2, 0.5};
	// icon->setSize({height, height});
	// float toolOptionsX = height;
	// float padding = height / 10;
	// float fileFontSize = height / 2;
	// float fileLineHeight = 0;
	// auto fileTextSize = font.stringSize(fileString, fileFontSize, fileLineHeight, {0, 0});
	// file->position = {toolOptionsX = (toolOptionsX + fileTextSize.x / 2 + padding), -height / 2, 0.5};
	// file->setSize(fileTextSize);
	// fileTextView->setSize(fileTextSize / 1.5f);
	// float editFontSize = height / 2;
	// float editLineHeight = 0;
	// auto editTextSize = font.stringSize(editString, editFontSize, editLineHeight, {0, 0});
	// edit->position = {toolOptionsX = (toolOptionsX + fileTextSize.x / 2 + editTextSize.x / 2 + padding), -height / 2, 0.5};
	// edit->setSize(editTextSize);
	// editTextView->setSize(editTextSize / 1.5f);
	// float helpFontSize = height / 2;
	// float helpLineHeight = 0;
	// auto helpTextSize = font.stringSize(helpString, helpFontSize, helpLineHeight, {0, 0});
	// help->position = {toolOptionsX = (toolOptionsX + editTextSize.x / 2 + helpTextSize.x / 2 + padding), -height / 2, 0.5};
	// help->setSize(helpTextSize);
	// helpTextView->setSize(helpTextSize / 1.5f);
	// scene.bvh.updateEntity(*xButton);
	// scene.bvh.updateEntity(*maxButton);
	// scene.bvh.updateEntity(*_Button);
	// scene.bvh.updateEntity(*this);
	// scene.bvh.updateEntity(*file);
	// scene.bvh.updateEntity(*edit);
	// scene.bvh.updateEntity(*help);
	// scene.bvh.updateEntity(*icon);
};

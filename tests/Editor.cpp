#include <iostream>
#include <anex/modules/gl/GLWindow.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
#include <anex/SharedLibrary.hpp>
#include <anex/RuntimeAPI.hpp>
#include <anex/modules/gl/entities/Toolbar.hpp>
#include <anex/modules/gl/entities/Panel.hpp>
using namespace anex;
using namespace anex::modules::gl;

static SharedLibrary gameLibrary("EditorGame.dll");

struct EditorScene : GLScene
{
	glm::vec4 editorClearColor = {0.2, 0.2, 0.2, 1};
	float toolbarHeight;
	File robotoRegularFile;
	modules::gl::fonts::freetype::FreetypeFont robotoRegularFont;
	GLWindow* childWindowPointer = 0;
	uint32_t childWindowWidth;
	uint32_t childWindowHeight;
	int32_t childWindowX;
	int32_t childWindowY;
	float childWindowBorderWidth = 4.f;
	std::shared_ptr<entities::Toolbar> toolbar;
	glm::vec4 childWindowBorderColor = {0.4, 0.4, 0.7, 1};
	glm::vec4 childWindowHoveredBorderColor = {0.7, 0.4, 0.4, 1};
	glm::vec4 childWindowActiveBorderColor = {1, 0, 0, 1};
	std::shared_ptr<entities::Plane> childWindowBorder;
	std::shared_ptr<entities::PanelMenu> sceneGraphPanelMenu;
	IWindow::EventIdentifier resizeID = 0;
	IWindow::EventIdentifier childWindowBorderHoverID = 0;
	IWindow::EventIdentifier childWindowBorderPressID = 0;
	IWindow::EventIdentifier childWindowESCPressID = 0;

	EditorScene(GLWindow& window):
		GLScene(window, {window.windowWidth / 2, window.windowHeight / 2, 50}, {0, 0, -1},
						{window.windowWidth, window.windowHeight}),
		toolbarHeight(window.windowHeight / 14),
		robotoRegularFile("fonts/Roboto/Roboto-Regular.ttf", enums::EFileLocation::Relative, "r"),
		robotoRegularFont(window, robotoRegularFile),
		childWindowWidth(window.windowWidth / 1.5f),
		childWindowHeight(window.windowHeight / 1.5f),
		childWindowX(window.windowWidth / 2 - childWindowWidth / 2),
		childWindowY(toolbarHeight + 4),
		toolbar(std::make_shared<entities::Toolbar>(
			window,
			*this,
			glm::vec3(0, window.windowHeight, 0),
			glm::vec3(0, 0, 0),
			glm::vec3(1, 1, 1),
			glm::vec4(35 / 255.f, 33 / 255.f, 36 / 255.f, 1),
			toolbarHeight,
			robotoRegularFont
		)),
		childWindowBorder(std::make_shared<entities::Plane>(
			window, *this,
			glm::vec3(childWindowX + (childWindowWidth / 2),
								window.windowHeight - childWindowY - (
									childWindowHeight / 2), 0.5), glm::vec3(0),
			glm::vec3(1),
			glm::vec2(childWindowWidth + childWindowBorderWidth * 2,
								childWindowHeight + childWindowBorderWidth * 2),
			childWindowBorderColor)),
		sceneGraphPanelMenu(std::make_shared<entities::PanelMenu>(
			window, *this,
			glm::vec3(0, window.windowHeight - toolbarHeight, 0),
			glm::vec3(0), glm::vec3(1), glm::vec4(0.5, 0.5, 0.5, 1),
			robotoRegularFont, "Scene Graph",
			(float)((window.windowWidth - childWindowWidth) / 2) - childWindowBorderWidth,
			childWindowHeight + childWindowBorderWidth * 2))
	{
		auto& childWindow = window.createChildWindow(
			"EditorChild",
			*this,
			childWindowWidth,
			childWindowHeight,
			childWindowX,
			childWindowY);
		childWindowPointer = (GLWindow*)&childWindow;
		childWindow.runOnThread([&](auto& window)
		{
			auto load = gameLibrary.getProc<LOAD_FUNCTION>("Load");
			load((GLWindow&)window);
		});
		std::function<void(const std::shared_ptr<IEntity>&)> entityAddedFunction = std::bind(
			&EditorScene::onEntityAdded, this, std::placeholders::_1);
		childWindow.registerOnEntityAddedFunction(entityAddedFunction);
		clearColor = editorClearColor;
		addEntity(toolbar);
		addEntity(childWindowBorder);
		sceneGraphPanelMenu->addToBVH = false;
		addEntity(sceneGraphPanelMenu);
		resizeID = view.addResizeHandler([&](auto& newSize)mutable
		{
			view.position = {newSize.x / 2, newSize.y / 2, 50};
			view.update();
			auto& toolbarRef = *toolbar;
			toolbarHeight = newSize.y / 14;
			toolbarRef.position.y = newSize.y;
			toolbarRef.setSize({newSize.x, toolbarHeight});
			auto& childWindow = *childWindowPointer;
			childWindowWidth = newSize.x / 2;
			childWindowHeight = newSize.y / 2;
			childWindow.setXY(newSize.x / 2 - childWindowWidth / 2, toolbarHeight);
			childWindow.setWidthHeight(childWindowWidth, childWindowHeight);
		});
		childWindowBorderHoverID = childWindowBorder->addMouseHoverHandler([&](const auto& hovered)
		{
			if (hovered)
				childWindowBorder->setColor(childWindowHoveredBorderColor);
			else
				childWindowBorder->setColor(childWindowBorderColor);
		});
		childWindowBorderPressID = childWindowBorder->addMousePressHandler(0, [&](const auto& pressed)
		{
			if (!pressed)
			{
				childWindowPointer->focused = true;
				if (currentHoveredEntity)
				{
					currentHoveredEntity = 0;
				}
				childWindowBorder->setColor(childWindowActiveBorderColor);
			}
		});
		childWindowESCPressID = childWindow.addKeyPressHandler(27, [&](const auto& pressed)
		{
			if (!pressed)
			{
				childWindowPointer->focused = false;
				window.callMouseMoveHandler(window.mouseCoords);
			}
		});
	};

	~EditorScene()
	{
		view.removeResizeHandler(resizeID);
		childWindowBorder->removeMouseHoverHandler(childWindowBorderHoverID);
		childWindowBorder->removeMousePressHandler(0, childWindowBorderPressID);
	};

	void onEntityAdded(const std::shared_ptr<IEntity>& entity)
	{
		auto& glEntity = *std::dynamic_pointer_cast<GLEntity>(entity);
		sceneGraphPanelMenu->addItem(glEntity.name, glEntity);
	}
};

int main()
{
	GLWindow window("Editor", 1280, 720, -1, -1, true);
	window.runOnThread([&](auto& window)mutable
	{
		auto editorScene = std::make_shared<EditorScene>((GLWindow&)window);
		window.setIScene(editorScene);
	});
	// while (!editorScene) {}
	// auto &editorSceneRef = *editorScene;
	// while (!editorSceneRef.childWindowPointer) {}
	// auto childWindowPointer = editorSceneRef.childWindowPointer;
	// editorScene.reset();
	// childWindowPointer->awaitWindowThread();
	window.awaitWindowThread();
};

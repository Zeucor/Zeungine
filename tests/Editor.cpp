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
	float toolbarHeight;
  File robotoRegularFile;
  modules::gl::fonts::freetype::FreetypeFont robotoRegularFont;
	GLWindow *childWindowPointer = 0;
	uint32_t childWindowWidth;
	uint32_t childWindowHeight;
	std::shared_ptr<entities::Toolbar> toolbar;
	std::shared_ptr<entities::PanelMenu> entityPanelMenu;
	IWindow::EventIdentifier resizeID = 0;
	EditorScene(GLWindow &window):
		GLScene(window, {window.windowWidth / 2, window.windowHeight / 2, 50}, {0, 0, -1}, {window.windowWidth, window.windowHeight}),
		toolbarHeight(window.windowHeight / 14),
    robotoRegularFile("fonts/Roboto/Roboto-Regular.ttf", enums::EFileLocation::Relative, "r"),
    robotoRegularFont(window, robotoRegularFile),
		childWindowWidth(window.windowWidth / 2),
		childWindowHeight(window.windowHeight / 2),
		toolbar(std::make_shared<entities::Toolbar>(window, *this, glm::vec3(0, window.windowHeight, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec4(35 / 255.f, 33 / 255.f, 36 / 255.f, 1), toolbarHeight, robotoRegularFont)),
		entityPanelMenu(std::make_shared<entities::PanelMenu>(window, *this, glm::vec3(0, window.windowHeight - toolbarHeight, 0), glm::vec3(0), glm::vec3(1), glm::vec4(0.5, 0.5, 0.5, 1), robotoRegularFont, "Scene Graph", (float)(window.windowWidth - childWindowWidth) / 2))
	{
		auto &childWindow = window.createChildWindow(
			"EditorChild",
			*this,
			childWindowWidth,
			childWindowHeight,
			window.windowWidth / 2 - childWindowWidth / 2, toolbarHeight);
		childWindowPointer = (GLWindow *)&childWindow;
		childWindow.runOnThread([&](auto &window)
		{
			auto load = gameLibrary.getProc<LOAD_FUNCTION>("Load");
			load((GLWindow &)window);
		});
		std::function<void(const std::shared_ptr<IEntity> &)> entityAddedFunction = std::bind(&EditorScene::onEntityAdded, this, std::placeholders::_1);
		childWindow.registerOnEntityAddedFunction(entityAddedFunction);
		clearColor = {0.2, 0.2, 0.2, 1};
		addEntity(toolbar);
		addEntity(entityPanelMenu);
		resizeID = view.addResizeHandler([&](auto &newSize)mutable
		{
			view.position = {newSize.x / 2, newSize.y / 2, 50};
			view.update();
			auto &toolbarRef = *toolbar;
			toolbarHeight = newSize.y / 14;
			toolbarRef.position.y = newSize.y;
			toolbarRef.setSize({ newSize.x, toolbarHeight });
			auto &childWindow = *childWindowPointer;
			childWindowWidth = newSize.x / 2;
			childWindowHeight = newSize.y / 2;
			childWindow.setXY(newSize.x / 2 - childWindowWidth / 2, toolbarHeight);
			childWindow.setWidthHeight(childWindowWidth, childWindowHeight);
		});
	};
	~EditorScene()
	{
		view.removeResizeHandler(resizeID);
	};
	void onEntityAdded(const std::shared_ptr<IEntity> &entity)
	{
		auto &glEntity = *std::dynamic_pointer_cast<GLEntity>(entity);
		entityPanelMenu->addItem(glEntity.name, glEntity);
	}
};
int main()
{
	GLWindow window("Editor", 1280, 720, -1, -1, true);
	window.runOnThread([&](auto &window)mutable
	{
		auto editorScene = std::make_shared<EditorScene>((GLWindow &)window);
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
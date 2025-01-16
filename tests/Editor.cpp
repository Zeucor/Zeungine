#include <anex/modules/gl/GLWindow.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
#include <anex/SharedLibrary.hpp>
#include <anex/RuntimeAPI.hpp>
#include <anex/modules/gl/entities/Toolbar.hpp>
#include <anex/modules/gl/raytracing/BVH.hpp>
using namespace anex;
using namespace anex::modules::gl;

static SharedLibrary gameLibrary("EditorGame.dll");
struct EditorScene : GLScene
{
	float toolbarHeight;
  File robotoRegularFile;
  modules::gl::fonts::freetype::FreetypeFont robotoRegularFont;
	std::shared_ptr<entities::Toolbar> toolbar;
	IWindow::EventIdentifier resizeID = 0;
	GLWindow *childWindowPointer = 0;
	uint32_t childWindowWidth;
	uint32_t childWindowHeight;
	EditorScene(GLWindow &window):
		GLScene(window, {window.windowWidth / 2, window.windowHeight / 2, 50}, {0, 0, -1}, {window.windowWidth, window.windowHeight}),
		toolbarHeight(window.windowHeight / 14),
    robotoRegularFile("fonts/Roboto/Roboto-Regular.ttf", enums::EFileLocation::Relative, "r"),
    robotoRegularFont(window, robotoRegularFile),
		toolbar(std::make_shared<entities::Toolbar>(window, *this, glm::vec3(0, window.windowHeight, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec4(35 / 255.f, 33 / 255.f, 36 / 255.f, 1), toolbarHeight, robotoRegularFont)),
		childWindowWidth(window.windowWidth / 2),
		childWindowHeight(window.windowHeight / 2)
	{
		auto &childWindow = window.createChildWindow(
			"EditorChild",
			childWindowWidth,
			childWindowHeight,
			window.windowWidth / 2 - childWindowWidth / 2, toolbarHeight);
		childWindowPointer = (GLWindow *)&childWindow;
		childWindow.runOnThread([&](auto &window)
		{
			auto load = gameLibrary.getProc<LOAD_FUNCTION>("Load");
			load((GLWindow &)window);
		});
		clearColor = {0.2, 0.2, 0.2, 1};
		auto toolbarID = addEntity(toolbar);
		toolbar->ID = toolbarID;
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
};
int main()
{
	GLWindow window("Editor", 1280, 720, -1, -1, true);
	std::shared_ptr<EditorScene> editorScene;
	window.runOnThread([&](auto &window)mutable
	{
		auto tempEditorScene = std::make_shared<EditorScene>((GLWindow &)window);
		window.setIScene(tempEditorScene);
		editorScene = tempEditorScene;
	});
	while (!editorScene) {}
	auto &editorSceneRef = *editorScene;
	while (!editorSceneRef.childWindowPointer) {}
	auto childWindowPointer = editorSceneRef.childWindowPointer;
	editorScene.reset();
	childWindowPointer->awaitWindowThread();
	window.awaitWindowThread();
};
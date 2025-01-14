#include <anex/modules/gl/GLWindow.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
#include <anex/SharedLibrary.hpp>
#include <anex/RuntimeAPI.hpp>
#include <anex/modules/gl/entities/Toolbar.hpp>
#include <anex/modules/gl/raytracing/BVH.hpp>
using namespace anex;
using namespace anex::modules::gl;

struct EditorScene : GLScene
{
	float toolbarHeight;
  File robotoRegularFile;
  modules::gl::fonts::freetype::FreetypeFont robotoRegularFont;
	std::shared_ptr<entities::Toolbar> toolbar;
	EditorScene(GLWindow &window, const float &toolbarHeight):
		GLScene(window, {window.windowWidth / 2, window.windowHeight / 2, 50}, {0, 0, -1}, {window.windowWidth, window.windowHeight}),
		toolbarHeight(toolbarHeight),
    robotoRegularFile("fonts/Roboto/Roboto-Regular.ttf", enums::EFileLocation::Relative, "r"),
    robotoRegularFont(window, robotoRegularFile),
		toolbar(std::make_shared<entities::Toolbar>(window, *this, glm::vec3(0, window.windowHeight, -10), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec4(0, 0, 0, 1), toolbarHeight, robotoRegularFont))
	{
		clearColor = {0.2, 0.2, 0.2, 1};
		addEntity(toolbar);
	};
};
int main()
{
	GLWindow window("Editor", 1280, 720, -1, -1, true);
	SharedLibrary gameLibrary("EditorGame.dll");
	auto load = gameLibrary.getProc<LOAD_FUNCTION>("Load");
	GLWindow *childWindowPointer = 0;
	window.runOnThread([&](auto &window)
	{
		float toolbarHeight = window.windowHeight / 18;
		auto editorScene = std::make_shared<EditorScene>((GLWindow &)window, toolbarHeight);
		window.setIScene(editorScene);
		raytracing::BVH bvh;
		bvh.triangles.push_back({
			{0, 0, 0},
			{window.windowWidth, 0, 0},
			{window.windowWidth / 2, window.windowHeight, 0}
		});
		bvh.buildBVH();
		auto childWindowWidth = 640;
		auto childWindowHeight = 480;
	  auto &childWindow = window.createChildWindow(
			"EditorChild",
			childWindowWidth,
			childWindowHeight,
window.windowWidth / 2 - childWindowWidth / 2,
			toolbarHeight);
		childWindowPointer = (GLWindow *)&childWindow;
		childWindow.runOnThread([&](auto &window)
		{
			load((GLWindow &)window);
		});
	});
	while (!childWindowPointer) {}
	childWindowPointer->awaitWindowThread();
	window.awaitWindowThread();
};
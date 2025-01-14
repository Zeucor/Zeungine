#include <anex/modules/gl/GLWindow.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
#include <anex/modules/gl/entities/TextView.hpp>
#include <anex/SharedLibrary.hpp>
#include <anex/RuntimeAPI.hpp>
using namespace anex;
using namespace anex::modules::gl;

// struct EditorScene : GLScene
// {
// 	File robotoRegularFile;
// 	std::shared_ptr<textures::Texture> texturePointer;
// 	modules::gl::fonts::freetype::FreetypeFont robotoRegularFont;
// 	EditorScene(GLWindow &window):
// 		GLScene(window, {0, 10, 10}, {0, -1, -1}, 81.f),
// 		robotoRegularFile("fonts/Roboto/Roboto-Regular.ttf", enums::EFileLocation::Relative, "r"),
// 		robotoRegularFont(window, robotoRegularFile)
// 	{
// 		clearColor = {0, 0, 1, 1};
// 		addEntity(std::make_shared<entities::TextView>(window, *this, glm::vec3(0, 5, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), "Test Text", glm::vec2( 47 / 3, 14 / 3), robotoRegularFont, 90.f));
// 	};
// };
int main()
{
	GLWindow window("Editor", 1280, 720, -1, -1);
	// window.runOnThread([](auto &window)
	// {
	// 	window.setIScene(std::make_shared<EditorScene>((GLWindow &)window));
	// });
	window.clearColor = {0, 0, 0, 1};
	SharedLibrary gameLibrary("EditorGame.dll");
	auto load = gameLibrary.getProc<LOAD_FUNCTION>("Load");
	GLWindow *childWindowPointer = 0;
	window.runOnThread([&](auto &window)
	{
		auto childWindowWidth = 640;
		auto childWindowHeight = 480;
	  auto &childWindow = window.createChildWindow(
			"EditorChild",
			childWindowWidth,
			childWindowHeight,
window.windowWidth / 2 - childWindowWidth / 2,
			0);
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
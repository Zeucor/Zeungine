#include <editor/Hotswapper.hpp>
#include <editor/EditorScene.hpp>
#include <anex/filesystem/Directory.hpp>
#include <anex/filesystem/File.hpp>
using namespace anex::editor::hs;
Hotswapper::Hotswapper(std::string_view directory, EditorScene &editorScene):
		running(true),
		directory(directory),
		editorScene(editorScene),
		swapper(std::make_shared<hscpp::Hotswapper>()),
		updateThread(std::make_shared<std::thread>(&Hotswapper::update, this))
{}
Hotswapper::~Hotswapper()
{
	running = false;
	updateThread->join();
}
void Hotswapper::update()
{
	auto &swapperRef = *swapper;
	auto srcDirectory = directory + "/src";
	filesystem::Directory srcDirectoryActual(srcDirectory);
	auto files = srcDirectoryActual.getRecursiveFileMap();
	for (auto &file : files)
	{
		swapperRef.AddForceCompiledSourceFile(file.second);
	}
	swapperRef.AddSourceDirectory(srcDirectory);
	swapperRef.AddIncludeDirectory(directory + "/include");
	auto programDirectoryPath = filesystem::File::getProgramDirectoryPath();
	swapperRef.AddIncludeDirectory(programDirectoryPath + "/../include");
	swapperRef.AddIncludeDirectory(programDirectoryPath + "/../vendor/glm");
	swapperRef.AddIncludeDirectory(programDirectoryPath + "/../vendor/stb");
	swapperRef.AddIncludeDirectory(programDirectoryPath + "/../vendor/bvh/src");
	swapperRef.AddIncludeDirectory(programDirectoryPath + "/../vendor/freetype/include");
	swapperRef.LocateAndAddLibrary(programDirectoryPath, "abstractnexus.lib");
	swapperRef.LinkLibrary("opengl32.lib");
	swapperRef.LinkLibrary("gdi32.lib");
	swapperRef.LinkLibrary("user32.lib");
	swapperRef.SetBuildDirectory(directory + "/build");
	while (!swapperRef.IsCompilerInitialized())
	{
		swapperRef.Update();
	}
	swapperRef.TriggerManualBuild(false);
	while (running)
	{
		auto updateResult = swapperRef.Update(false);
		if (updateResult == hscpp::Hotswapper::UpdateResult::BeforeCompile)
		{
			if (editorScene.loaded)
			{
				if (editorScene.OnUnLoad)
				{
					editorScene.OnUnLoad(*editorScene.gameWindowPointer);
				}
				editorScene.loaded = false;
				editorScene.OnLoad = std::function<void(GLWindow&)>();
				editorScene.OnUnLoad = std::function<void(GLWindow&)>();
				swapperRef.UnLoadModule();
			}
		}
		else if (updateResult == hscpp::Hotswapper::UpdateResult::Compiled)
		{
			editorScene.OnLoad = swapperRef.GetFunction<void(GLWindow &)>("OnLoad");
			editorScene.OnUnLoad = swapperRef.GetFunction<void(GLWindow &)>("OnUnLoad");
			if (editorScene.OnLoad)
			{
				editorScene.OnLoad(*editorScene.gameWindowPointer);
				editorScene.loaded = true;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
Hotswapper &Hotswapper::operator=(const Hotswapper &hotswapper)
{
	running = hotswapper.running;
	directory = hotswapper.directory;
	swapper = hotswapper.swapper;
	updateThread = hotswapper.updateThread;
	compiling = hotswapper.compiling;
	return *this;
};
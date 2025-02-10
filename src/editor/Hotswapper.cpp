#include <zg/editor/Hotswapper.hpp>
#include <zg/editor/EditorScene.hpp>
#include <zg/filesystem/Directory.hpp>
#include <zg/filesystem/File.hpp>
#include <zg/system/CommandExecutor.hpp>
#include <zg/system/WorkingDirectory.hpp>
#include <zg/filesystem/DirectoryWatcher.hpp>
#include <zg/SharedLibrary.hpp>
using namespace zg::editor::hs;
using namespace zg::system;
Hotswapper::Hotswapper(const std::filesystem::path& directory, EditorScene &editorScene) : running(true),
																			   directory(directory),
																			   editorScene(editorScene),
																			   updateThread(std::make_shared<std::thread>(&Hotswapper::update, this))
{
}
Hotswapper::~Hotswapper()
{
	running = false;
	updateThread->join();
}
void Hotswapper::update()
{
	filesystem::DirectoryWatcher directoryWatcher(directory, {
		directory / "build",
		directory / "cmake"
	});
	editorScene.status->setText("Configuring...");
	editorScene.status->setTextColor({1, 1, 0, 1});
	idle = false;
	bool requireConfigure = true,
		requireBuild = false,
		requireLoad = false;
	std::unique_ptr<SharedLibrary> libraryPointer;
	while (running)
	{
		if (requireConfigure)
		{
			requireBuild = configure();
			requireConfigure = false;
		}
		if (requireBuild)
		{
			requireLoad = build();
			requireBuild = false;
		}
		if (requireLoad)
		{
			if (editorScene.loaded)
			{
				{
					std::lock_guard gameWindowLock(editorScene.gameWindowPointer->renderMutex);
					editorScene.gameWindowPointer->scene.reset();
				}
				editorScene.loaded = false;
				editorScene.OnLoad = 0;
				libraryPointer.reset();
			}
			libraryPointer = std::make_unique<SharedLibrary>(directory / "build" / "libeditor-game.so");
			auto& libraryRef = *libraryPointer;
			try
			{
				editorScene.OnLoad = libraryRef.getProc<void(*)(Window &)>("OnLoad");
			}
			catch(const std::exception& e)
			{
				std::cout << e.what() << '\n';
			}
			if (editorScene.OnLoad)
			{
				editorScene.OnLoad(*editorScene.gameWindowPointer);
				editorScene.loaded = true;
				if (editorScene.gameWindowPointer->minimized)
				{
					editorScene.gameWindowPointer->restore();
				}
			}
			requireLoad = false;
		}
		auto changes = directoryWatcher.update();
		if (!changes.empty())
		{
			for (auto& changePair : changes)
			{
				if (strcmp(changePair.second.c_str(), "CMakeLists.txt") == 0)
					requireConfigure = true;
			}
			if (!requireConfigure)
				requireBuild = true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
bool Hotswapper::configure()
{
	auto currentWorkingDirectory = GET_WORKING_DIR();
	SET_WORKING_DIR(directory.c_str());
	auto exitStatus = CommandExecutor::execute("cmake -B build .");
	SET_WORKING_DIR(currentWorkingDirectory.c_str());
	return !exitStatus;
};
bool Hotswapper::build()
{
	editorScene.status->setText("Building...");
	editorScene.status->setTextColor({1, 1, 0, 1});
	compiling = true;
	compiled = false;
	auto currentWorkingDirectory = GET_WORKING_DIR();
	SET_WORKING_DIR(directory.c_str());
	auto exitStatus = CommandExecutor::execute("cmake --build build");
	SET_WORKING_DIR(currentWorkingDirectory.c_str());
	std::cout << "Compile finished with status: " << exitStatus << std::endl;
	if (exitStatus)
	{
		compiling = false;
		errored = true;
		editorScene.status->setText("Build Error");
		editorScene.status->setTextColor({1, 0, 0, 1});
	}
	else
	{
		compiling = false;
		compiled = true;
		editorScene.status->setText("Idle");
		editorScene.status->setTextColor({1, 1, 1, 1});
		idle = true;
	}
	return !exitStatus;
};
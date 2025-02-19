#include <zg/SharedLibrary.hpp>
#include <zg/editor/EditorScene.hpp>
#include <zg/editor/Hotswapper.hpp>
#include <zg/zgfilesystem/Directory.hpp>
#include <zg/zgfilesystem/DirectoryWatcher.hpp>
#include <zg/zgfilesystem/File.hpp>
#include <zg/system/WorkingDirectory.hpp>
#include <zg/system/Budget.hpp>
#include <zg/crypto/Random.hpp>
using namespace zg::editor::hs;
using namespace zg::system;
SECONDS_DURATION hotswapperBudgetDuration = SECONDS_DURATION(1.0 * nano::den);
zg::budget::ZBudget hotswapperZBudget(hotswapperBudgetDuration);
Hotswapper::Hotswapper(const std::filesystem::path& directory, EditorScene& editorScene) :
		running(true), directory(directory), editorScene(editorScene),
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
	zgfilesystem::DirectoryWatcher directoryWatcher(directory, {directory / "build"});
	bool requireConfigure = true, requireBuild = false, requireLoad = false, currentlyConfiguring = false,
			 currentlyBuilding = false;
	std::unique_ptr<SharedLibrary> libraryPointer;
	while (running)
	{
		hotswapperZBudget.update();
		auto configureResult = configure(currentlyConfiguring, requireConfigure);
		if (justRequireConfigure && configureResult.first && configureResult.second)
		{
			requireBuild = true;
		}
		auto buildResult = build(currentlyBuilding, requireBuild);
		if (justRequireBuild && buildResult.first && buildResult.second)
		{
			requireLoad = true;
		}
		if (requireLoad)
		{
			editorScene.gameWindowPointer->runOnThread([&](auto& _window)
			{
				if (editorScene.loaded)
				{
					{
						std::lock_guard gameWindowLock(_window.renderMutex);
						_window.scene.reset();
					}
					editorScene.loaded = false;
					editorScene.OnLoad = 0;
					libraryPointer.reset();
				}
				libraryPointer = std::make_unique<SharedLibrary>(directory / "build" / "libeditor-game.so");
				auto& libraryRef = *libraryPointer;
				try
				{
					editorScene.OnLoad = libraryRef.getProc<void (*)(Window&)>("OnLoad");
				}
				catch (const std::exception& e)
				{
					std::cout << e.what() << '\n';
				}
				if (editorScene.OnLoad)
				{
					editorScene.OnLoad(_window);
					editorScene.loaded = true;
					if (_window.minimized)
					{
						_window.restore();
					}
				}
				requireLoad = false;
			});
		}
		auto changes = directoryWatcher.update();
		if (!changes.empty())
		{
			for (auto& changePair : changes)
			{
				if (changePair.second.find("CMakeLists.txt") != std::string::npos || changePair.second.find(".cmake") != std::string::npos)
					requireConfigure = true;
			}
			if (!requireConfigure)
				requireBuild = true;
			// if (currentCommand)
			// 	currentCommand->kill();
		}
		hotswapperZBudget.update();
		std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::seconds, REAL>(SECONDS_DURATION(zg::crypto::Random::value<REAL>(0.021, 0.042))));
		hotswapperZBudget.update();
		hotswapperZBudget.zsleep();
	}
}
std::pair<bool, bool> Hotswapper::configure(bool& currentlyConfiguring, bool& requireConfigure)
{
	if ((currentlyConfiguring && requireConfigure))
	{
		currentCommand->kill();
	}
	if (requireConfigure)
	{
		editorScene.status->setTextColor({1, 1, 0, 1});
		editorScene.status->setText("Configuring...");
		idle = false;
		auto currentWorkingDirectory = GET_WORKING_DIR();
		SET_WORKING_DIR(directory.c_str());
		currentCommand = std::make_unique<Command>("cmake -B build .");
		SET_WORKING_DIR(currentWorkingDirectory.c_str());
		currentlyConfiguring = true;
	}
	if (requireConfigure)
	{
		justRequireConfigure = true;
		requireConfigure = false;
	}
	if (!currentlyConfiguring)
		return {false, false};
	auto& currentCommandRef = *currentCommand;
	currentCommandRef.update();
	auto complete = currentCommandRef.isComplete();
	auto exitCode = currentCommandRef.getExitCode();
	if (complete)
	{
		currentCommandRef.update();
		currentCommand.reset();
		currentlyConfiguring = false;
	}
	return {complete, !exitCode};
};
std::pair<bool, bool> Hotswapper::build(bool& currentlyBuilding, bool& requireBuild)
{
	if ((currentlyBuilding && requireBuild))
	{
		currentCommand->kill();
	}
	if (requireBuild)
	{
		editorScene.status->setTextColor({1, 1, 0, 1});
		editorScene.status->setText("Building...");
		compiling = true;
		compiled = false;
		auto currentWorkingDirectory = GET_WORKING_DIR();
		SET_WORKING_DIR(directory.c_str());
		if (currentCommand)
			currentCommand->update();
		currentCommand = std::make_unique<Command>("cmake --build build");
		SET_WORKING_DIR(currentWorkingDirectory.c_str());
		currentlyBuilding = true;
	}
	if (requireBuild)
	{
		justRequireBuild = true;
		requireBuild = false;
	}
	if (!currentlyBuilding)
		return {false, false};
	auto& currentCommandRef = *currentCommand;
	currentCommandRef.update();
	auto complete = currentCommandRef.isComplete();
	auto exitCode = currentCommandRef.getExitCode();
	if (complete)
	{
		currentCommandRef.update();
		currentCommand.reset();
		currentlyBuilding = false;
		if (exitCode)
		{
			compiling = false;
			errored = true;
			editorScene.status->setTextColor({1, 0, 0, 1});
			editorScene.status->setText("Build Error");
		}
		else
		{
			compiling = false;
			compiled = true;
			editorScene.status->setTextColor({1, 1, 1, 1});
			editorScene.status->setText("Idle");
			idle = true;
		}
	}
	return {complete, !exitCode};
};

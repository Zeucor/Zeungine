#pragma once
#include <filesystem>
#include <memory>
#include <thread>
#include <zg/system/Command.hpp>
namespace zg::editor
{
	struct EditorScene;
}
namespace zg::editor::hs
{
	struct Hotswapper
	{
		bool running = false;
		std::filesystem::path directory;
		EditorScene& editorScene;
		// std::shared_ptr<hscpp::Hotswapper> swapper;
		std::shared_ptr<std::thread> updateThread;
		bool compiling = false;
		bool compiled = false;
		bool idle = true;
		bool errored = false;
		bool justRequireConfigure;
		bool justRequireBuild;
		std::chrono::time_point<std::chrono::system_clock> compiledTime = std::chrono::system_clock::now();
		std::unique_ptr<system::Command> currentCommand;
		Hotswapper() = delete;
		Hotswapper(const std::filesystem::path& directory, EditorScene& editorScene);
		~Hotswapper();
		void update();
		std::pair<bool, bool> configure(bool& currentlyConfiguring, bool& requireConfigure);
		std::pair<bool, bool> build(bool& currentlyBuilding, bool& requireBuild);
	};
} // namespace zg::editor::hs

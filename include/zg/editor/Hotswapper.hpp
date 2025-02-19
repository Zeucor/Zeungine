#pragma once
#include <filesystem>
#include <memory>
#include <thread>
#include <zg/system/Command.hpp>
using namespace std;
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
		// shared_ptr<hscpp::Hotswapper> swapper;
		shared_ptr<thread> updateThread;
		bool compiling = false;
		bool compiled = false;
		bool idle = true;
		bool errored = false;
		bool justRequireConfigure;
		bool justRequireBuild;
		chrono::time_point<chrono::system_clock> compiledTime = chrono::system_clock::now();
		unique_ptr<system::Command> currentCommand;
		Hotswapper() = delete;
		Hotswapper(const std::filesystem::path& directory, EditorScene& editorScene);
		~Hotswapper();
		void update();
		pair<bool, bool> configure(bool& currentlyConfiguring, bool& requireConfigure);
		pair<bool, bool> build(bool& currentlyBuilding, bool& requireBuild);
	};
} // namespace zg::editor::hs

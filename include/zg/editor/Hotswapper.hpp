#pragma once
#include <memory>
#include <thread>
#include <filesystem>
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
		EditorScene &editorScene;
		// std::shared_ptr<hscpp::Hotswapper> swapper;
		std::shared_ptr<std::thread> updateThread;
		bool compiling = false;
		bool compiled = false;
		bool idle = true;
		bool errored = false;
		std::chrono::time_point<std::chrono::system_clock> compiledTime = std::chrono::system_clock::now();
		Hotswapper() = delete;
		Hotswapper(const std::filesystem::path& directory, EditorScene &editorScene);
		~Hotswapper();
		void update();
		bool configure();
		bool build();
	};
}
#pragma once
#include <hscpp/Hotswapper.h>
#include <thread>
namespace anex::editor
{
	struct EditorScene;
}
namespace anex::editor::hs
{
	struct Hotswapper
	{
		bool running = false;
		std::string directory;
		EditorScene &editorScene;
		std::shared_ptr<hscpp::Hotswapper> swapper;
		std::shared_ptr<std::thread> updateThread;
		bool compiling = false;
		bool compiled = false;
		bool idle = true;
		std::chrono::time_point<std::chrono::system_clock> compiledTime = std::chrono::system_clock::now();
		Hotswapper() = delete;
		Hotswapper(std::string_view directory, EditorScene &editorScene);
		~Hotswapper();
		void update();
	};
}
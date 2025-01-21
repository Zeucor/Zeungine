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
		Hotswapper() = default;
		Hotswapper(std::string_view directory, EditorScene &editorScene);
		~Hotswapper();
		Hotswapper &operator=(const Hotswapper &);
		void update();
	};
}
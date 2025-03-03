#include <iostream>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/media/entities/Video.hpp>
#include <zg/zgfilesystem/File.hpp>
using namespace zg;
struct VideoScene : Scene
{
	long double deltaTimeCounter = 0;
	zg::EventIdentifier qPressID = 0;
	VideoScene(Window& _window) :
			Scene(_window, {0, 0, 1}, // camera position
						glm::normalize(glm::vec3(0, 0, -1)), // camera direction
						{2, 2}) // using NDC
	{
		clearColor = {1, 0, 1, 1};
		// auto mediaPath = zgfilesystem::File::getProgramDirectoryPath() / "music" / "Kalimba.mp3";
		// auto mediaPath = zgfilesystem::File::getProgramDirectoryPath() / "videos" / "ForBiggerJoyrides.mp4";
		// auto mediaPath = zgfilesystem::File::getProgramDirectoryPath() / "videos" / "WeAreGoingOnBullrun.mp4";
		// auto mediaPath = zgfilesystem::File::getProgramDirectoryPath() / "videos" / "ForBiggerFun.mp4";
		// auto mediaPath = zgfilesystem::File::getProgramDirectoryPath() / "videos" / "ForBiggerBlazes.mp4";
		auto mediaPath = zgfilesystem::File::getProgramDirectoryPath() / "videos" / "VolkswagenGTIReview.mp4";
		auto video = std::make_shared<media::entities::Video>(
			window, // reference to window
			*this, // reference to scene
			glm::vec3(0, 0, 0), // position
			glm::vec3(0, 0, 0), // rotation
			glm::vec3(1, 1, 1), // scale
			glm::vec2(2, 2), // cube size
			mediaPath.string(),
			std::make_shared<zgfilesystem::File>(mediaPath, enums::EFileLocation::Absolute, "r"));
		addEntity(video);
		qPressID = window.addKeyPressHandler('q',
																				 [&](auto pressed)
																				 {
																					 if (pressed)
																						 window.close();
																				 });
		auto videoStage = std::make_shared<zg::audio::AudioStage>(window.audioEngine);
		videoStage->addSoundNode(video);
		window.audioEngine.pipeline.addStage(videoStage);
	}
	~VideoScene() { window.removeKeyPressHandler('q', qPressID); }
	void update() override {}
};
int main()
{
	Window window("Video Test", 1024, 768, -1, -1, true, false);
	window.runOnThread([](auto& window) { window.setScene(std::make_shared<VideoScene>(window)); });
	window.run();
}

#include <iostream>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/media/entities/Video.hpp>
#include <zg/vp/VML.hpp>
using namespace zg;
struct VideoScene : Scene
{
	vp::VML vml; // view mouse look
	long double deltaTimeCounter = 0;
	uint32_t qPressID = 0;
	VideoScene(Window& _window) :
			Scene(_window, {0, 10, 10}, // camera position
						glm::normalize(glm::vec3(0, -1, -1)), // camera direction
						{2, 2}),
			vml(*this)
	{
		clearColor = {1, 0, 1, 1};
		auto video = std::make_shared<media::entities::Video>(
			window, // reference to window
			*this, // reference to scene
			glm::vec3(0, 0, 0), // position
			glm::vec3(0, 0, 0), // rotation
			glm::vec3(1, 1, 1), // scale
			glm::vec2(2, 2), // cube size
			"http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4");
		addEntity(video);
		qPressID = window.addKeyPressHandler('q',
																				 [&](auto pressed)
																				 {
																					 if (pressed)
																						 window.close();
																				 });
		window.setXY(320, 320);
		auto videoStage = std::make_shared<zg::audio::AudioStage>(window.audioEngine);
		videoStage->addSoundNode(video);
		window.audioEngine.pipeline.addStage(videoStage);
	}
	~VideoScene() { window.removeKeyPressHandler('q', qPressID); }
	void update() override {}
};
int main()
{
	Window window("Cube Test", 1024, 768, -1, -1, true, false);
	window.runOnThread([](auto& window) { window.setScene(std::make_shared<VideoScene>(window)); });
	window.run();
}

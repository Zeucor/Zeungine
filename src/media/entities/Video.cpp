#include <zg/Scene.hpp>
#include <zg/media/entities/Video.hpp>
using namespace zg::media::entities;
Video::Video(Window& _window, Scene& _scene, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale,
						 glm::vec2 _size, const std::string& uri) :
		Entity(_window, {"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}, 6,
					 {0, 1, 2, 2, 3, 0}, 4, {}, position, rotation, scale,
					 name.empty() ? "Video " + std::to_string(++videosCount) : name),
		InputMediaStream(_window, uri),
		ISoundNode(_window.audioEngine, true, false),
		scene(_scene)
{
	setSize(glm::vec3(_size, 0));
}
Video::Video(Window& _window, Scene& _scene, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale,
						 glm::vec2 _size, const std::string& uri, const std::shared_ptr<interfaces::IFile>& _filePointer) :
		Entity(_window, {"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}, 6,
					 {0, 1, 2, 2, 3, 0}, 4, {}, position, rotation, scale,
					 name.empty() ? "Video " + std::to_string(++videosCount) : name),
		InputMediaStream(_window, uri, _filePointer),
		ISoundNode(_window.audioEngine, true, false),
		scene(_scene)
{
	setSize(glm::vec3(_size, 0));
}
bool Video::preRender()
{
	fillTexture(*texturePointer);
	const auto& model = getModelMatrix();
	shader->bind(*this);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, scene.view.matrix);
	shader->setBlock("Projection", *this, scene.projection.matrix);
	shader->setBlock("CameraPosition", *this, scene.view.position, 16);
	shader->setTexture("Texture2D", *this, *texturePointer, 0);
	shader->unbind();
	return true;
}
void Video::setSize(glm::vec3 newSize)
{
	size = newSize;
	positions = {{0, -size.y, 0}, {size.x, -size.y, 0}, {size.x, 0, 0}, {0, 0, 0}};
	updateElements("Position", positions);
}
std::vector<float> Video::inputFrames(const float *frames, const int32_t &channelCount, const unsigned long &frameCount, const zg::audio::audio_time_t &time)
{
}
void Video::outputFrames(float *frames, const int32_t &channelCount, const unsigned long &frameCount, const zg::audio::audio_time_t &time)
{
	fillAudioFrames(frames, channelCount, frameCount);
}
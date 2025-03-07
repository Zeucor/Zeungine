#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/media/entities/Video.hpp>
using namespace zg::media::entities;
Video::Video(Window& _window, Scene& _scene, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale,
						 glm::vec2 _size, const std::string& uri) :
		Entity(_window, {"UV2", "Texture2D", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}, 6,
					 {0, 1, 2, 2, 3, 0}, 4, {}, _position, _rotation, _scale,
					 name.empty() ? "Video " + std::to_string(++videosCount) : name),
		ReadMediaStream(_window, uri), ISoundNode(_window.audioEngine, true, false), scene(_scene), uvs({
																																																	// Front face
																																																	{0, 0}, // 0
																																																	{1, 0}, // 1
																																																	{1, 1}, // 2
																																																	{0, 1} // 3
																																																})
{
	init(_size);
}
Video::Video(Window& _window, Scene& _scene, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale,
						 glm::vec2 _size, const std::string& uri, const std::shared_ptr<interfaces::IFile>& _filePointer) :
		Entity(_window, {"UV2", "Texture2D", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}, 6,
					 {0, 1, 2, 2, 3, 0}, 4, {}, _position, _rotation, _scale,
					 name.empty() ? "Video " + std::to_string(++videosCount) : name),
		ReadMediaStream(_window, uri, _filePointer), ISoundNode(_window.audioEngine, true, false), scene(_scene),
		uvs({
			// Front face
			{0, 0}, // 0
			{1, 0}, // 1
			{1, 1}, // 2
			{0, 1} // 3
		})
{
	init(_size);
}
void Video::init(glm::vec2 _size)
{
	std::cerr << "opening video: " << uri << std::endl;
	switch (window.iRenderer->renderer)
	{
	default:
		break;
	case RENDERER_VULKAN:
	case RENDERER_METAL:
		flipUVsY(uvs);
		break;
	}
	updateIndices(indices);
	updateElements("UV2", uvs);
	setSize(glm::vec3(_size, 0));
	computeNormals(indices, positions, normals);
	updateElements("Normal", normals);
	NANOSECONDS_DURATION videoFrameDuration = getVideoFrameDuration();
	budget = std::make_shared<ZBudget<>>(videoFrameDuration, (size_t)1, true, false, uri + "+budget");
}
bool Video::preRender()
{
	sweetFrameTime = budget->begin();
	if (sweetFrameTime)
	{
		if (firstFrame)
		{
			window.addPreSwapbuffersOnceoff([&]()
			{
				startAudio();
			});
			firstFrame = false;
		}
		fillNextVideoFrame(texturePointer);
		budget->end();
	}
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
	positions = {{-size.x / 2, -size.y / 2, 0}, {size.x / 2, -size.y / 2, 0}, {size.x / 2, size.y / 2, 0}, {-size.x / 2, size.y / 2, 0}};
	updateElements("Position", positions);
}
std::vector<float> Video::inputFrames(const float* frames, const int32_t& channelCount, const unsigned long& frameCount,
																			const zg::audio::audio_time_t& time)
{
	return {};
}
void Video::outputFrames(float* frames, const int32_t& channelCount, const unsigned long& frameCount,
												 const zg::audio::audio_time_t& time)
{
	fillNextAudioFrames(frames, channelCount, frameCount);
}

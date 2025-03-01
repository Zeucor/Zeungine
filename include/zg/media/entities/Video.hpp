#pragma once
#include <zg/Entity.hpp>
#include <zg/media/AudioDecoder.hpp>
#include <zg/media/InputMediaStream.hpp>
#include <zg/media/VideoDecoder.hpp>
#include <zg/interfaces/ISizable.hpp>
namespace zg::media::entities
{
	struct Video : Entity, ISizable, zg::media::InputMediaStream, zg::audio::ISoundNode
	{
		Scene& scene;
		std::shared_ptr<textures::Texture> texturePointer;
		inline static size_t videosCount = 0;
		Video(Window& _window, Scene& _scene, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale, glm::vec2 _size,
					const std::string& uri);
		Video(Window& _window, Scene& _scene, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale, glm::vec2 _size,
					const std::string& uri, const std::shared_ptr<interfaces::IFile>& filePointer);
		bool preRender() override;
		void setSize(glm::vec3 newSize) override;
		std::vector<float> inputFrames(const float *frames, const int32_t &channelCount, const unsigned long &frameCount, const zg::audio::audio_time_t &time);
		void outputFrames(float *frames, const int32_t &channelCount, const unsigned long &frameCount, const zg::audio::audio_time_t &time);
	};
} // namespace zg::entities::media

#pragma once
#include <zg/Entity.hpp>
#include <zg/interfaces/ISizable.hpp>
#include <zg/media/AudioDecoder.hpp>
#include <zg/media/ReadMediaStream.hpp>
#include <zg/media/VideoDecoder.hpp>
#include <zg/system/Budget.hpp>
namespace zg::media::entities
{
	struct Video : Entity, ISizable, zg::media::ReadMediaStream, zg::audio::ISoundNode
	{
		Scene& scene;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals = {};
		std::shared_ptr<textures::Texture> texturePointer;
		std::shared_ptr<budget::ZBudget<>> budget;
		bool sweetFrameTime = true;
		inline static size_t videosCount = 0;
		Video(Window& _window, Scene& _scene, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale, glm::vec2 _size,
					const std::string& uri, std::string_view name = "");
		Video(Window& _window, Scene& _scene, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale, glm::vec2 _size,
					const std::string& uri, const std::shared_ptr<interfaces::IFile>& filePointer, std::string_view name = "");

	private:
		bool firstFrame = true;
		void init(glm::vec2 _size);

	public:
		bool preRender() override;
		void setSize(glm::vec3 newSize) override;
		std::vector<float> inputFrames(const float* frames, const int32_t& channelCount, const unsigned long& frameCount,
																	 const zg::audio::audio_time_t& time);
		void outputFrames(float* frames, const int32_t& channelCount, const unsigned long& frameCount,
											const zg::audio::audio_time_t& time);
	};
} // namespace zg::media::entities

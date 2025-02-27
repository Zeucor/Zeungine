#pragma once
#include <zg/Entity.hpp>
#include <zg/media/AudioDecoder.hpp>
#include <zg/media/InputMediaStream.hpp>
#include <zg/media/VideoDecoder.hpp>
namespace zg::entities::media
{
	struct Video : Entity, zg::media::InputMediaStream, IAudioDecoder, IVideoDecoder
	{
		Video(Window& _window, Scene& _scene, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale,
					const std::string& uri);
		Video(Window& _window, Scene& _scene, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale,
					const std::string& uri, const std::shared_ptr<interfaces::IFile>& filePointer);
	};
} // namespace zg::entities::media

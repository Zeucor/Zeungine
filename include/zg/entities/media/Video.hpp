#pragma once
#include <zg/Entity.hpp>
#include <zg/interfaces/media/IAudioDecoder.hpp>
#include <zg/interfaces/media/IMediaStream.hpp>
#include <zg/interfaces/media/IVideoDecode.hpp
namespace zg::entities::media
{
	struct Video : Entity, IMediaStream, IAudioDecoder, IVideoDecoder
	{
		Video(Window& _window, Scene& _scene, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale);
	};
} // namespace zg::entities::media

#pragma once
#include "audio_time_t.hpp"
#include <vector>
namespace zg::audio
{
    struct AudioEngine;
    struct ISoundNode
	{
        AudioEngine& audioEngine;
		bool isOutput = false;
		bool isInput = false;
		bool isActive = false;
		audio_time_t activationTime = 0;
		audio_time_t deactivationTime = 0;
		ISoundNode(AudioEngine& _audioEngine, const bool &isOutput, const bool &isInput);
		virtual ~ISoundNode() = default;
		virtual std::vector<float> inputFrames(const float *frames, const int32_t &channelCount, const unsigned long &frameCount, const audio_time_t &time) = 0;
		virtual void outputFrames(float *frames, const int32_t &channelCount, const unsigned long &frameCount, const audio_time_t &time) = 0;
		void activate();
		void deactivate();
	};
}
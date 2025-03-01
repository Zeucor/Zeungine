#pragma once
#include "AudioStage.hpp"
namespace zg::audio
{
    struct AudioEngine;
	struct AudioPipeline : ISoundNode
	{
		std::map<uint64_t, std::shared_ptr<AudioStage>> stages;
		std::recursive_mutex stagesMutex;
		uint64_t stagesCount = 0;
		AudioPipeline(AudioEngine& _audioEngine);
		std::vector<float> inputFrames(const float* frames, const int32_t& channelCount, const unsigned long& frameCount,
																	 const audio_time_t& time) override;
		void outputFrames(float* frames, const int32_t& channelCount, const unsigned long& frameCount,
											const audio_time_t& time) override;
		uint64_t addStage(const std::shared_ptr<AudioStage>& stagePointer, int64_t index = -1);
		bool removeStage(uint64_t& index);
	};
} // namespace zg::audio

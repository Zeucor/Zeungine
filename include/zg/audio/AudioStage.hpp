#pragma once
#include <map>
#include <memory>
#include <mutex>
#include "ISoundNode.hpp"
namespace zg::audio
{
    struct AudioEngine;
	struct AudioStage : ISoundNode
	{
		std::map<uint64_t, std::shared_ptr<ISoundNode>> soundNodes;
		std::recursive_mutex soundNodesMutex;
		uint64_t soundNodesCount = 0;
		AudioStage(AudioEngine& _audioEngine);
		std::vector<float> inputFrames(const float* frames, const int32_t& channelCount, const unsigned long& frameCount,
																	 const audio_time_t& time) override;
		void outputFrames(float* frames, const int32_t& channelCount, const unsigned long& frameCount,
											const audio_time_t& time) override;
		uint64_t addSoundNode(const std::shared_ptr<ISoundNode>& soundNodePointer, int64_t index = -1);
		bool removeSoundNode(uint64_t& index);
	};
} // namespace zg::audio

#pragma once
#include "AudioPipeline.hpp"
#define MINIAUDIO_IMPLEMENTATION
#define MA_API inline static
#include <miniaudio.h>
namespace zg::audio
{
    struct AudioEngine
    {
		AudioPipeline pipeline;
		ma_format format = ma_format_f32;
		ma_uint32 inputChannels = 2;
		ma_uint32 outputChannels = 2;
		ma_uint32 sampleRate = 48000;
		int32_t frameSize = 64;
		ma_resource_manager_config resourceManagerConfig;
		ma_resource_manager resourceManager;
		ma_device_config deviceConfig;
		ma_device device;
		ma_engine engine;
		ma_engine_config engineConfig;
		ma_context context;
		uint64_t startTimeNs;
		bool running = false;
		AudioEngine();
		~AudioEngine();
		static void audioCallback(ma_device *pDevice, void *pOutput, const void *pInput, uint32_t frameCount);
		int32_t start();
		ma_device_config getDefaultDeviceConfig(ma_device_info *playbackDeviceInfos, const ma_uint32 &playbackDeviceCount, int32_t &selectedDeviceIndex);
		int32_t stop();
		bool await();
		audio_time_t getEngineTime();
        audio_time_t getTimeNanoSeconds();
    };
}
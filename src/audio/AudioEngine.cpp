#include <zg/audio/AudioEngine.hpp>
#include <iostream>
#include <chrono>
using namespace zg::audio;
AudioEngine::AudioEngine():
    pipeline(*this)
{
	start();
}
AudioEngine::~AudioEngine()
{
    stop();
}
void AudioEngine::audioCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
	AudioEngine *audioEnginePointer = (AudioEngine *)pDevice->pUserData;
	if (audioEnginePointer == 0)
	{
		return;
	}
	if (pInput)
	{
		auto time = audioEnginePointer->getEngineTime();
		audioEnginePointer->pipeline.inputFrames((const float *)pInput, audioEnginePointer->inputChannels, frameCount, time);
	}
	if (pOutput)
	{
		auto time = audioEnginePointer->getEngineTime();
		audioEnginePointer->pipeline.outputFrames((float *)pOutput, audioEnginePointer->outputChannels, frameCount, time);
	}
}
int32_t AudioEngine::start()
{
	ma_result result;

	resourceManagerConfig = ma_resource_manager_config_init();
	resourceManagerConfig.decodedFormat = format;
	resourceManagerConfig.decodedChannels = outputChannels;
	resourceManagerConfig.decodedSampleRate = sampleRate;

	result = ma_resource_manager_init(&resourceManagerConfig, &resourceManager);
	if (result != MA_SUCCESS)
	{
		std::cerr << "Failed to initialize resource manager.";
		return -1;
	}

	result = ma_context_init(0, 0, 0, &context);
	if (result != MA_SUCCESS)
	{
		std::cerr << "Failed to initialize context.";
		return -2;
	}

	ma_device_info *playbackDeviceInfos = 0;
	ma_uint32 playbackDeviceCount = 0;
	int32_t selectedDeviceIndex = -1;

	result = ma_context_get_devices(&context, &playbackDeviceInfos, &playbackDeviceCount, NULL, NULL);
	if (result != MA_SUCCESS)
	{
		std::cerr << "Failed to enumerate playback devices.";
		ma_context_uninit(&context);
		return -3;
	}

	deviceConfig = getDefaultDeviceConfig(playbackDeviceInfos, playbackDeviceCount, selectedDeviceIndex);

	result = ma_device_init(nullptr, &deviceConfig, &device);
	if (result != MA_SUCCESS)
	{
		std::cerr << "Failed to initialize playback device.";
		return -4;
	}

	engineConfig = ma_engine_config_init();
	engineConfig.pDevice = &device;
	engineConfig.pResourceManager = &resourceManager;
	engineConfig.noAutoStart = MA_TRUE;

	result = ma_engine_init(&engineConfig, &engine);
	if (result != MA_SUCCESS)
	{
		std::cerr << "Failed to initialize engine for %s.\n", playbackDeviceInfos[selectedDeviceIndex].name;
		ma_device_uninit(&device);
		return -5;
	}

	result = ma_engine_start(&engine);
	if (result != MA_SUCCESS)
	{
		std::cerr << "Failed to start engine.";
        return -6;
	}

	startTimeNs = getTimeNanoSeconds();

	running = true;

	return 0;
}
ma_device_config AudioEngine::getDefaultDeviceConfig(ma_device_info *playbackDeviceInfos, const ma_uint32 &playbackDeviceCount, int32_t &selectedDeviceIndex)
{
	for (int32_t index = 0; index < playbackDeviceCount; index++)
	{
		if (playbackDeviceInfos[index].isDefault)
		{
			selectedDeviceIndex = index;
		}
	}
	if (selectedDeviceIndex == -1 && playbackDeviceCount)
	{
		selectedDeviceIndex = 0;
	}
	ma_device_config _deviceConfig = ma_device_config_init(ma_device_type_playback);
	if (selectedDeviceIndex != -1)
	{
		_deviceConfig.playback.pDeviceID = &playbackDeviceInfos[selectedDeviceIndex].id;
	}
	_deviceConfig.playback.format = resourceManager.config.decodedFormat;
	_deviceConfig.playback.channels = resourceManager.config.decodedChannels;
	_deviceConfig.sampleRate = resourceManager.config.decodedSampleRate;
	_deviceConfig.dataCallback = audioCallback;
	_deviceConfig.pUserData = this;
	_deviceConfig.periodSizeInFrames = frameSize;
	return _deviceConfig;
}
int32_t AudioEngine::stop()
{
	if (!running)
	{
		return -1;
	}
	ma_device_uninit(&device);
	ma_engine_uninit(&engine);
	ma_context_uninit(&context);
	ma_resource_manager_uninit(&resourceManager);
	running = false;
	return 0;
}
bool AudioEngine::clearPipeline()
{
	pipeline.stages.clear();
	pipeline.stagesCount = 0;
	return false;
}
audio_time_t AudioEngine::getEngineTime()
{
	return getTimeNanoSeconds() - startTimeNs;
}
audio_time_t AudioEngine::getTimeNanoSeconds()
{
    return std::chrono::system_clock::now().time_since_epoch().count();
}
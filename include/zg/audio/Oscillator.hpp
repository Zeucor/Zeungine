#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>
#include <vector>
#include <zg/audio/AudioEngine.hpp>
#include <zg/audio/ISoundNode.hpp>
#include <zg/glm.hpp>
namespace zg::audio
{
	struct WaveParameters
	{
		double frequencyMultiplier = 1.0; // Multiplies the base oscillator frequency
		double amplitudeMultiplier = 1.0; // Multiplies the base oscillator amplitude
		double phaseOffset = 0.0; // Adds a phase offset (in radians)
		double pulseWidth = 0.5; // For square wave (0.0 to 1.0)
		double asymmetry = 0.0; // For sawtooth/triangle (-1.0 to 1.0)
	};
	struct Oscillator : public ISoundNode
	{
		static float sineWave(double currentPhase, double baseAmplitude, const WaveParameters& params)
		{
			double currentAmplitude = baseAmplitude * params.amplitudeMultiplier;
			return static_cast<float>(currentAmplitude * std::sin(currentPhase + params.phaseOffset));
		}

		static float squareWave(double currentPhase, double baseAmplitude, const WaveParameters& params)
		{
			double currentAmplitude = baseAmplitude * params.amplitudeMultiplier;
			double normalizedPhase = fmod((currentPhase + params.phaseOffset) / (2.0 * ZG_PI), 1.0);
			if (normalizedPhase < 0)
				normalizedPhase += 1.0;

			double pulseWidth = std::clamp(params.pulseWidth, 0.0, 1.0);

			return static_cast<float>(currentAmplitude * (normalizedPhase < pulseWidth ? 1.0 : -1.0));
		}

		static float sawtoothWave(double currentPhase, double baseAmplitude, const WaveParameters& params)
		{
			double currentAmplitude = baseAmplitude * params.amplitudeMultiplier;
			// Apply phase offset and normalize phase to [0, 1) for shape calculation
			double normalizedPhase = fmod((currentPhase + params.phaseOffset) / (2.0 * ZG_PI), 1.0);
			if (normalizedPhase < 0)
				normalizedPhase += 1.0;

			// A simpler linear sawtooth: rises linearly from -1 to 1
			double value = -1.0 + 2.0 * normalizedPhase;

			// Asymmetry implementation (can be complex, using a simpler linear blend approach for now)
			double asymmetry = std::clamp(params.asymmetry, -1.0, 1.0);
			if (asymmetry != 0.0)
			{
				// Simple linear interpolation based on asymmetry
				// More complex asymmetry would involve piecewise functions
				if (normalizedPhase < (1.0 + asymmetry) / 2.0)
				{
					value = -1.0 + 2.0 * normalizedPhase / ((1.0 + asymmetry) / 2.0);
				}
				else
				{
					value = 1.0 - 2.0 * (normalizedPhase - (1.0 + asymmetry) / 2.0) / ((1.0 - asymmetry) / 2.0);
				}
			}


			return static_cast<float>(currentAmplitude * value);
		}

		static float triangleWave(double currentPhase, double baseAmplitude, const WaveParameters& params)
		{
			double currentAmplitude = baseAmplitude * params.amplitudeMultiplier;
			// Apply phase offset and normalize phase to [0, 1) for shape calculation
			double normalizedPhase = fmod((currentPhase + params.phaseOffset) / (2.0 * ZG_PI), 1.0);
			if (normalizedPhase < 0)
				normalizedPhase += 1.0;

			// Asymmetry parameter shifts the peak location
			double asymmetry = std::clamp(params.asymmetry, -1.0, 1.0); // -1: peak at start, 1: peak at end, 0: peak at 0.5
			double peakPos = 0.5 + asymmetry * 0.5; // Peak position in [0, 1]

			double value;
			if (normalizedPhase < peakPos)
			{
				value = -1.0 + 2.0 * (normalizedPhase / peakPos);
			}
			else
			{
				value = 1.0 - 2.0 * ((normalizedPhase - peakPos) / (1.0 - peakPos));
			}

			// Handle edge case where peakPos is exactly 0 or 1
			if (peakPos == 0.0 && normalizedPhase != 0.0)
				value = -1.0; // At 0, value is 1, otherwise -1
			if (peakPos == 1.0 && normalizedPhase != 1.0)
				value = -1.0; // At 1, value is 1, otherwise -1
			if (peakPos == 0.0 && normalizedPhase == 0.0)
				value = 1.0;
			if (peakPos == 1.0 && normalizedPhase == 1.0)
				value = 1.0;


			return static_cast<float>(currentAmplitude * value);
		}
		double frequency; // Base frequency in Hz
		double amplitude; // Base amplitude (0.0 to 1.0)
		using WaveGeneratorFunc =
			std::function<float(double currentPhase, double baseAmplitude, const WaveParameters& params)>;
		struct LayeredWave
		{
			WaveGeneratorFunc func;
			WaveParameters params;
			double currentPhase = 0.0;
		};

	private:
		std::vector<LayeredWave> layeredWaves;

	public:
		// Constructor
		Oscillator(AudioEngine& _audioEngine, double freq = 440.0, double amp = 0.5) :
				ISoundNode(_audioEngine, true, false), frequency(freq), amplitude(amp)
		{
		}
		void addWaveLayer(WaveGeneratorFunc func, const WaveParameters& params) { layeredWaves.push_back({func, params}); }
		void removeWaveLayer(size_t index)
		{
			if (index < layeredWaves.size())
			{
				layeredWaves.erase(layeredWaves.begin() + index);
			}
		}
		// Modify parameters of an existing wave layer
		bool modifyWaveLayerParams(size_t index, const WaveParameters& newParams)
		{
			if (index < layeredWaves.size())
			{
				layeredWaves[index].params = newParams;
				return true;
			}
			return false;
		}
		std::vector<float> inputFrames(const float* frames, const int32_t& channelCount, const unsigned long& frameCount,
																	 const audio_time_t& time) override
		{
			return {};
		}

		void outputFrames(float* frames, const int32_t& channelCount, const unsigned long& frameCount,
											const audio_time_t& time) override
		{
			if (!isActive)
			{
				std::fill(frames, frames + frameCount * channelCount, 0.0f);
				return;
			}

			double sampleRate = audioEngine.sampleRate;

			for (unsigned long i = 0; i < frameCount; ++i)
			{
				float sampleValue = 0.0f;
				double basePhaseIncrement = (frequency / sampleRate) * 2.0 * ZG_PI;
				for (auto& wave : layeredWaves)
				{
					double layerFrequency = frequency * wave.params.frequencyMultiplier;
					double layerPhaseIncrement = (layerFrequency / sampleRate) * 2.0 * ZG_PI;
					sampleValue += wave.func(wave.currentPhase, amplitude, wave.params);
					wave.currentPhase += layerPhaseIncrement;
					wave.currentPhase = fmod(wave.currentPhase, 2.0 * ZG_PI);
					if (wave.currentPhase < 0)
					{
						wave.currentPhase += 2.0 * ZG_PI;
					}
				}
				sampleValue = std::clamp(sampleValue, -1.0f, 1.0f);
				for (int32_t j = 0; j < channelCount; ++j)
				{
					frames[i * channelCount + j] = sampleValue;
				}
			}
		}
	};
} // namespace zg::audio

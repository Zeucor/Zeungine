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
		static float sineWave(double currentPhase, double baseAmplitude, const WaveParameters& params);
		static float squareWave(double currentPhase, double baseAmplitude, const WaveParameters& params);
		static float sawtoothWave(double currentPhase, double baseAmplitude, const WaveParameters& params);
		static float triangleWave(double currentPhase, double baseAmplitude, const WaveParameters& params);
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
		double currentEnvelopeLevel = 0.0; // Current amplitude multiplier from envelope
		double envelopeTargetLevel = 0.0;  // Target level (0.0 or 1.0)
		double envelopeStepPerSample = 0.0; // Amount to change envelope level per sample
		double envelopeDuration = 0.01; // Duration of fade in/out in seconds (e.g., 10ms)

	public:
		// Constructor
		Oscillator(AudioEngine& _audioEngine, double freq = 440.0, double amp = 0.5);
		void addWaveLayer(WaveGeneratorFunc func, const WaveParameters& params);
		void removeWaveLayer(size_t index);
		bool modifyWaveLayerParams(size_t index, const WaveParameters& newParams);
		std::vector<float> inputFrames(const float* frames, const int32_t& channelCount, const unsigned long& frameCount,
																	 const audio_time_t& time) override;
		void outputFrames(float* frames, const int32_t& channelCount, const unsigned long& frameCount,
											const audio_time_t& time) override;
		bool activateVirtual() override;
		bool deactivateVirtual() override;
	};
} // namespace zg::audio

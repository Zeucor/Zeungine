#include <zg/Window.hpp>
#include <zg/Registry.hpp>
#include <zg/audio/MIDIEngine.hpp>
#include <zg/audio/Oscillator.hpp>
using namespace zg;
using namespace zg::audio;
int main()
{
    WindowCreateInfo windowInfo{
        .title = "MIDI Test",
        .windowWidth = 1024,
        .windowHeight = 768,
        .framerate = 60
    };
    auto window_tuple = Registry::addWindow(windowInfo);
    auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);
    auto oscillatorStage = std::make_shared<zg::audio::AudioStage>(window.audioEngine);
    auto oscillator = std::make_shared<zg::audio::Oscillator>(
        window.audioEngine,
        440,
        0.5
    );
    oscillator->addWaveLayer(Oscillator::sineWave, {});
    oscillatorStage->addSoundNode(oscillator);
    window.audioEngine.pipeline.addStage(oscillatorStage);
    MIDIEngine midiEngine(true);
    midiEngine.addEventHandler([&](MIDIEvent& event) {
        switch (event.type)
        {
        case MIDIEventType::NoteOn:
        {
            oscillator->activate();
            double midiNote = static_cast<double>(event.data1);
            double frequency = 440.0 * std::pow(2.0, (midiNote - 69.0) / 12.0);
            oscillator->frequency = frequency;
            break;
        }
        case MIDIEventType::NoteOff:
            oscillator->deactivate();
            break;
        }
    });
    window.run();
}
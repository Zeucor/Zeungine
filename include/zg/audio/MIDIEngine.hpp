#pragma once
#include <RtMidi.h>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>
namespace zg::audio
{
	enum class MIDIEventType
	{
		NoteOff = 0x80, // 128
		NoteOn = 0x90, // 144
		PolyphonicKeyPressure = 0xA0, // 160
		ControlChange = 0xB0, // 176
		ProgramChange = 0xC0, // 192
		ChannelPressure = 0xD0, // 208
		PitchBendChange = 0xE0, // 224
		SystemExclusive = 0xF0, // 240
		TimeCode = 0xF1,
		SongPositionPointer = 0xF2,
		SongSelect = 0xF3,
		TuneRequest = 0xF6,
		EndOfExclusive = 0xF7,
		TimingClock = 0xF8,
		Start = 0xFA,
		Continue = 0xFB,
		Stop = 0xFC,
		ActiveSensing = 0xFE,
		SystemReset = 0xFF,
		Invalid = 0x00 // Add an invalid type.
	};
	MIDIEventType getMIDIEventType(unsigned char statusByte);
	struct MIDIEvent
	{
		MIDIEventType type;
		uint8_t channel; // 0-15
		uint8_t data1;
		uint8_t data2;
		double timestamp;
		MIDIEvent() : type(MIDIEventType::Invalid), channel(0), data1(0), data2(0), timestamp(0.0) {}
		MIDIEvent(MIDIEventType t, uint8_t ch, uint8_t d1, uint8_t d2, double ts) :
				type(t), channel(ch), data1(d1), data2(d2), timestamp(ts)
		{}
		void print() const
		{
			std::cout << "MIDIEvent - Type: ";
			switch (type)
			{
			case MIDIEventType::NoteOff:
				std::cout << "NoteOff";
				break;
			case MIDIEventType::NoteOn:
				std::cout << "NoteOn";
				break;
			case MIDIEventType::PolyphonicKeyPressure:
				std::cout << "PolyphonicKeyPressure";
				break;
			case MIDIEventType::ControlChange:
				std::cout << "ControlChange";
				break;
			case MIDIEventType::ProgramChange:
				std::cout << "ProgramChange";
				break;
			case MIDIEventType::ChannelPressure:
				std::cout << "ChannelPressure";
				break;
			case MIDIEventType::PitchBendChange:
				std::cout << "PitchBendChange";
				break;
			case MIDIEventType::SystemExclusive:
				std::cout << "SystemExclusive";
				break;
			case MIDIEventType::TimeCode:
				std::cout << "TimeCode";
				break;
			case MIDIEventType::SongPositionPointer:
				std::cout << "SongPositionPointer";
				break;
			case MIDIEventType::SongSelect:
				std::cout << "SongSelect";
				break;
			case MIDIEventType::TuneRequest:
				std::cout << "TuneRequest";
				break;
			case MIDIEventType::EndOfExclusive:
				std::cout << "EndOfExclusive";
				break;
			case MIDIEventType::TimingClock:
				std::cout << "TimingClock";
				break;
			case MIDIEventType::Start:
				std::cout << "Start";
				break;
			case MIDIEventType::Continue:
				std::cout << "Continue";
				break;
			case MIDIEventType::Stop:
				std::cout << "Stop";
				break;
			case MIDIEventType::ActiveSensing:
				std::cout << "ActiveSensing";
				break;
			case MIDIEventType::SystemReset:
				std::cout << "SystemReset";
				break;
			case MIDIEventType::Invalid:
				std::cout << "Invalid";
				break;
			default:
				std::cout << "Unknown";
				break;
			}
			std::cout << ", Channel: " << static_cast<int>(channel) << ", Data1: " << static_cast<int>(data1)
								<< ", Data2: " << static_cast<int>(data2) << ", Timestamp: " << std::fixed << std::setprecision(3)
								<< timestamp // Consistent formatting
								<< std::endl;
		}
	};
    using MIDIEventHandler = std::function<void(MIDIEvent&)>;
	struct MIDIEngine
	{
		std::vector<RtMidi::Api> apis;
		std::vector<RtMidiIn> midiIns;
		std::vector<RtMidiOut> midiOuts;
		std::map<int32_t, std::string> apiMap;
		bool initialized = false;
        std::map<size_t, MIDIEventHandler> eventHandlers;
		MIDIEngine(bool connect = true);
		bool initialize(bool connect);
		bool update();
		uint32_t getNumInPorts(size_t apiIndex);
		std::string getInPortName(size_t apiIndex, size_t port);
		size_t getNumOutPorts(size_t apiIndex);
		std::string getOutPortName(size_t apiIndex, size_t port);
		void openInPort(size_t apiIndex, size_t port, RtMidiIn::RtMidiCallback messageCallback,
										RtMidiErrorCallback errorCallback);
		void openOutPort(size_t apiIndex, size_t port, RtMidiErrorCallback errorCallback);
		void closeInPort(size_t apiIndex);
		void closeOutPort(size_t apiIndex);
        size_t addEventHandler(const MIDIEventHandler& handler);
        bool removeEventHandler(size_t& ID);
		static void defaultLoggingCallback(double deltatime, std::vector<unsigned char>* message, void* userData);
		static void defaultErrorCallback(RtMidiError::Type type, const std::string& errorText, void* userData);
	};
} // namespace zg::audio

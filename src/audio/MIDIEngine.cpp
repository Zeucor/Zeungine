#include <iomanip>
#include <iostream>
#include <zg/audio/MIDIEngine.hpp>
#include <zg/GlobalUID.hpp>
using namespace zg::audio;
MIDIEventType zg::audio::getMIDIEventType(unsigned char statusByte)
{
	if (statusByte >= 0x80 && statusByte <= 0xEF)
	{ // Covers Note Off through Pitch Bend
		return static_cast<MIDIEventType>(statusByte & 0xF0); // Get the high nibble.
	}
	else if (statusByte >= 0xF0)
	{
		return static_cast<MIDIEventType>(statusByte);
	}
	return MIDIEventType::Invalid;
}
MIDIEngine::MIDIEngine(bool connect) { initialize(connect); }
bool MIDIEngine::initialize(bool connect)
{
	if (initialized)
	{
		return false;
	}
	apiMap[RtMidi::MACOSX_CORE] = "OS-X CoreMIDI";
	apiMap[RtMidi::WINDOWS_MM] = "Windows MultiMedia";
	apiMap[RtMidi::WINDOWS_UWP] = "Windows UWP";
	apiMap[RtMidi::UNIX_JACK] = "Jack Client";
	apiMap[RtMidi::LINUX_ALSA] = "Linux ALSA";
	apiMap[RtMidi::RTMIDI_DUMMY] = "RtMidi Dummy";
	update();
	auto apisSize = apis.size();
	std::cout << "Available MIDI apis:" << std::endl;
	for (size_t i = 0; i < apisSize; i++)
	{
		std::cout << apiMap[apis[i]].c_str() << std::endl;
	}
	std::cout << std::endl;
	auto numInPorts = getNumInPorts(0);
	std::cout << "Number of connected in ports: " << numInPorts << std::endl;
	if (numInPorts > 0)
	{
		auto portName = getInPortName(0, 0);
		std::cout << "Opening port: " << portName.c_str() << std::endl;
		openInPort(0, 0, defaultLoggingCallback, defaultErrorCallback);
	}
	return initialized = true;
}
bool MIDIEngine::update()
{
	for (auto& midiIn : midiIns)
		if (midiIn.isPortOpen())
			midiIn.closePort();
	for (auto& midiOut : midiOuts)
		if (midiOut.isPortOpen())
			midiOut.closePort();
	apis.clear();
	midiIns.clear();
	midiOuts.clear();
	RtMidi::getCompiledApi(apis);
	for (auto& api : apis)
	{
		midiIns.emplace_back(api);
		midiOuts.emplace_back(api);
	}
	return true;
}
uint32_t MIDIEngine::getNumInPorts(size_t apiIndex)
{
	auto& midiIn = midiIns[apiIndex];
	return midiIn.getPortCount();
}
std::string MIDIEngine::getInPortName(size_t apiIndex, size_t port)
{
	auto& midiIn = midiIns[apiIndex];
	return midiIn.getPortName(port).c_str();
}
size_t MIDIEngine::getNumOutPorts(size_t apiIndex)
{
	auto& midiOut = midiOuts[apiIndex];
	return midiOut.getPortCount();
}
std::string MIDIEngine::getOutPortName(size_t apiIndex, size_t port)
{
	auto& midiOut = midiOuts[apiIndex];
	return midiOut.getPortName(port).c_str();
}
void MIDIEngine::openInPort(size_t apiIndex, size_t port, RtMidiIn::RtMidiCallback messageCallback,
														RtMidiErrorCallback errorCallback)
{
	auto& midiIn = midiIns[apiIndex];
	if (midiIn.isPortOpen())
		throw std::runtime_error("MidiIn port already open");
	midiIn.openPort(port);
	midiIn.setCallback(messageCallback, this);
	midiIn.setErrorCallback(errorCallback, this);
}
void MIDIEngine::openOutPort(size_t apiIndex, size_t port, RtMidiErrorCallback errorCallback)
{
	auto& midiOut = midiOuts[apiIndex];
	if (midiOut.isPortOpen())
		throw std::runtime_error("MidiOut port already open");
	midiOut.openPort(port);
	midiOut.setErrorCallback(errorCallback, this);
}
void MIDIEngine::closeInPort(size_t apiIndex)
{
	auto& midiIn = midiIns[apiIndex];
	midiIn.closePort();
}
void MIDIEngine::closeOutPort(size_t apiIndex)
{
	auto& midiOut = midiOuts[apiIndex];
	midiOut.closePort();
}
size_t MIDIEngine::addEventHandler(const MIDIEventHandler& handler)
{
    auto ID = GlobalUID::GetNew();
    eventHandlers[ID] = handler;
    return ID;
}
bool MIDIEngine::removeEventHandler(size_t& ID)
{
    auto iter = eventHandlers.find(ID);
    if (iter == eventHandlers.end())
        return false;
    eventHandlers.erase(iter);
    ID = 0;
    return true;
}
void MIDIEngine::defaultLoggingCallback(double deltatime, std::vector<unsigned char>* message, void* userData)
{
    auto& midiEngine = *(MIDIEngine*)userData;
	size_t nBytes = message->size();
	MIDIEvent event;
	if (nBytes > 0)
	{
		event.type = getMIDIEventType(message->at(0));
		event.timestamp = deltatime;

		// Extract the channel.  Important!
		if (event.type >= MIDIEventType::NoteOff && event.type <= MIDIEventType::PitchBendChange)
		{
			event.channel = message->at(0) & 0x0F; // Get lower 4 bits.
		}
		else
		{
			event.channel = 0; // Default channel.
		}

		if (nBytes > 1)
		{
			event.data1 = message->at(1);
		}
		if (nBytes > 2)
		{
			event.data2 = message->at(2);
		}
		for (auto& handlerPair : midiEngine.eventHandlers)
            handlerPair.second(event);
	}
	else
	{
		std::cout << "Empty MIDI message received." << std::endl;
	}
};
void MIDIEngine::defaultErrorCallback(RtMidiError::Type type, const std::string& errorText, void* userData)
{
	std::cerr << "MIDI error: " << errorText.c_str() << std::endl;
};

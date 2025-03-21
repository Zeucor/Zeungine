#include <zg/zgfilesystem/Serial.hpp>
#include <fstream>
#include <string>
template<>
zgfilesystem::Serial& zgfilesystem::deserialize(Serial& serial, std::string& str)
{
    auto size = str.size();
    serial >> size;
    str.resize(size);
    serial.readBytes(str.data(), size);
    return serial;
}
template<>
zgfilesystem::Serial& zgfilesystem::serialize(Serial& serial, const std::string& str)
{
    auto size = str.size();
    serial << size;
    serial.writeBytes(str.c_str(), size);
    return serial;
}
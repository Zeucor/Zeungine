#include <zg/Serial.hpp>
#include <string>

template <>
Serial& deserialize(Serial& serial, std::string& str)
{
    auto size = str.size();
    serial >> size;
    str.resize(size);
    serial.readBytes(str.data(), size);
    return serial;
}
template<>
Serial& serialize(Serial& serial, const std::string& str)
{
    auto size = str.size();
    serial << size;
    serial.writeBytes(str.c_str(), size);
    return serial;
}
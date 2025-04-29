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
template <>
Serial& deserialize(Serial& serial, std::vector<std::string>& vec)
{
    auto size = vec.size();
    serial >> size;
    vec.resize(size);
    for (auto i = 0; i < size; ++i)
        serial >> vec[i];
    return serial;
}
template <>
Serial& serialize(Serial& serial, const std::vector<std::string>& vec)
{
    auto size = vec.size();
    serial << size;
    for (auto i = 0; i < size; ++i)
        serial << vec[i];
    return serial;
}
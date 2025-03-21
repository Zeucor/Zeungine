#include <zg/zgfilesystem/Serial.hpp>
#include <fstream>
namespace zgfilesystem
{
    template <typename WriteStreamT, typename ReadStreamT>
    Serial<WriteStreamT, ReadStreamT>& deserialize(Serial<WriteStreamT, ReadStreamT>& serial, std::string& str)
    {
        auto size = str.size();
        serial >> size;
        str.resize(size);
        serial.readBytes(str.data(), size);
        return serial;
    }
    template <typename WriteStreamT, typename ReadStreamT>
    Serial<WriteStreamT, ReadStreamT>& serialize(Serial<WriteStreamT, ReadStreamT>& serial, const std::string& str)
    {
        auto size = str.size();
        serial << size;
        serial.writeBytes(str.c_str(), size);
        return serial;
    }
}
template zgfilesystem::Serial<std::fstream, std::fstream>& zgfilesystem::deserialize(Serial<std::fstream, std::fstream>&, std::string &);
template zgfilesystem::Serial<std::fstream, std::fstream>& zgfilesystem::serialize(Serial<std::fstream, std::fstream>&, const std::string &);
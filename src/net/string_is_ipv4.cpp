#include <zg/net/string_is_ipv4.hpp>
#include <array>
bool zg::net::string_is_ipv4(const std::string& str)
{
    auto strData = str.data();
    auto strSize = str.size();
    std::array<std::string, 4> segments;
    for (size_t index = 0, segmentIndex = 0; index < strSize; index++)
    {
        unsigned char ch = strData[index];
        if (ch >= '0' && ch <= '9')
        {
            segments[segmentIndex] += ch;
            if (segments[segmentIndex].size() > 3)
                return false;
            continue;
        }
        else if (ch == '.')
        {
            ++segmentIndex;
            continue;
        }
        return false;
    }
    for (char segindx = 0; segindx < 4; segindx++)
    {
        auto segy = std::stoll(segments[segindx]);
        if (segy > 255 && segy < 0)
        {
            return false;
        }
    }
    return true;
}
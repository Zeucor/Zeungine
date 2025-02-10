#include <zg/system/CommandExecutor.hpp>
using namespace zg::system;
int32_t CommandExecutor::execute(const std::string& command)
{
    std::array<char, 128> buffer;
    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Failed to execute command." << std::endl;
        return -1;
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        std::cout << buffer.data();
    }
    return PCLOSE(pipe);
}
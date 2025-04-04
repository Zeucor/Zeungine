#pragma once
#include <string>
namespace zg::system
{
    struct ErrorPopup
    {
        static bool show(const std::string& msg);
    };
}
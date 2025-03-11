#pragma once
#include <zg/Standard.hpp>
#include <zg/zgfilesystem/File.hpp>
namespace zg
{
    struct Window;
}
namespace zg::fonts
{
    struct SystemFonts
    {
        Window& window;
        std::unordered_map<std::string, std::filesystem::path> fontPaths;
        SystemFonts(Window& _window);
        void refresh();
        template<typename T>
        T queryForFont(const std::string &fontquery)
        {
            auto q_iter = fontPaths.find(fontquery);
            if (q_iter == fontPaths.end())
            {
                throw std::runtime_error("queried Font not found!");
            }
            zgfilesystem::File fontFile(q_iter->second, enums::EFileLocation::Absolute, "r");
            return T(window, fontFile);
        }
    };
}
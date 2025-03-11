#include <zg/fonts/SystemFonts.hpp>
#include <zg/zgfilesystem/Directory.hpp>
using namespace zg::fonts;
SystemFonts::SystemFonts(Window& _window):
    window(_window)
{
    refresh();
}
void SystemFonts::refresh()
{
    static constexpr uint32_t fontDirsLength =
    #if defined(WIN32)
        1;
    #elif defined(__linux__)
        1;
    #elif defined(MACOS)
        1;
    #elif defined(ANDROID)
        1;
    #elif defined(IOS)
        1;
    #endif
    std::filesystem::path fontDirs[fontDirsLength] = {
#if defined(WIN32)
        "C:\\Windows\\Fonts"
#elif defined(__linux__)
        "/usr/share/fonts/"
#elif defined(MACOS)
        "/System/Library/Fonts/"
#elif defined(ANDROID)
        "/system/fonts/"
#elif defined(IOS)
        "/System/Library/Fonts/"
#endif
    };
    for (auto fontDir : fontDirs)
    {
        zgfilesystem::Directory fontDirectory(fontDir);
        auto fontRecursiveFileMap = fontDirectory.getRecursiveFileMap();
        for (auto& path : fontRecursiveFileMap)
        {
            if (std::filesystem::is_regular_file(path.second))
            {
                auto filename = path.second.filename().string();
                if (path.second.has_extension())
                {
                    auto ext = path.second.extension().string();
                    filename = filename.substr(0, filename.size() - ext.size());
                }
                fontPaths[filename] = path.second;
            }
        }
    }
}
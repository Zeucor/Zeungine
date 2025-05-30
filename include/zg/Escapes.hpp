#pragma once
#include <zg/glm.hpp>
namespace zg
{
    struct Escapes
    {
        static std::string Color(glm::vec3 color, bool foreground = true);
        static std::string ResetAttr;
        static std::string FG_Black;
        static std::string FG_Red;
        static std::string FG_Green;
        static std::string FG_Yellow;
        static std::string FG_Blue;
        static std::string FG_Magenta;
        static std::string FG_Cyan;
        static std::string FG_White;
        static std::string FG_BrightBlack;
        static std::string FG_BrightRed;
        static std::string FG_BrightGreen;
        static std::string FG_BrightYellow;
        static std::string FG_BrightBlue;
        static std::string FG_BrightMagenta;
        static std::string FG_BrightCyan;
        static std::string FG_BrightWhite;
        static std::string BG_Black;
        static std::string BG_Red;
        static std::string BG_Green;
        static std::string BG_Yellow;
        static std::string BG_Blue;
        static std::string BG_Magenta;
        static std::string BG_Cyan;
        static std::string BG_White;
        static std::string BG_BrightBlack;
        static std::string BG_BrightRed;
        static std::string BG_BrightGreen;
        static std::string BG_BrightYellow;
        static std::string BG_BrightBlue;
        static std::string BG_BrightMagenta;
        static std::string BG_BrightCyan;
        static std::string BG_BrightWhite;
        static const glm::vec4 ansiStandardColors[8];
        static const glm::vec4 ansiBrightColors[8];
        static glm::vec4 resolveAnsiColor(int code);
        static std::string BoldFont;
        static std::string SmallerFont;
        static std::string LargerFont;
    };
}
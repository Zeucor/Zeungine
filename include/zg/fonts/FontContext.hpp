#pragma once
#include <zg/glm.hpp>
namespace zg::fonts
{
    struct FontContext
    {
        float fontSize;
        float originalFontSize;
        float lineHeight;
        float x;
        float y;
        float start_line_x;
        glm::vec4 foreground_color;
        glm::vec4 background_color;
        glm::vec4 original_foreground_color;
        glm::vec4 original_background_color;
        std::function<void(FontContext&)> notify_font_size_changed;
    };
}
#include <zg/fonts/ParseFontEscapes.hpp>
#include <utility>
#include <cctype>
#include <sstream>
#include <algorithm>
#include <zg/Escapes.hpp>
zg::fonts::ParsedEscapePair zg::fonts::parseFontEscapes(std::string_view rawString)
{
  std::string cleaned;
  IndexedEscapeContextKeyFunctionsMap indexedFunctions;

  size_t i = 0;

  while (i < rawString.size())
  {
    if (rawString[i] == '\x1b' && i + 1 < rawString.size() && rawString[i + 1] == '[')
    {
      size_t start = i;
      size_t end = i + 2;

      // Move to 'm' or end of string
      while (end < rawString.size() && rawString[end] != 'm')
      {
        ++end;
      }

      if (end >= rawString.size())
      {
        break; // malformed or unterminated
      }

      ++end; // include 'm'
      auto sequence = std::string(rawString.substr(start, end - start));
      i = end;

      // Check if it's a truecolor escape: \x1b[38;2;r;g;b or \x1b[48;2;r;g;b
      if (sequence.starts_with("\x1b[38;2;") || sequence.starts_with("\x1b[48;2;"))
      {
        bool isForeground = sequence[2] == '3'; // 38 = FG, 48 = BG
        size_t prefixLen = 7;

        auto colorPart = sequence.substr(prefixLen, sequence.size() - prefixLen - 1); // remove \x1b[38;2; and trailing 'm'
        std::stringstream ss(colorPart);
        std::string component;
        int rgb[3] = {0, 0, 0};
        int idx = 0;

        while (std::getline(ss, component, ';') && idx < 3)
        {
          rgb[idx++] = std::clamp(std::stoi(component), 0, 255);
        }

        glm::vec3 color = glm::vec3(rgb[0], rgb[1], rgb[2]) / 255.0f;

        indexedFunctions[cleaned.size()].push_back({sequence, [color, isForeground](FontContext& ctx)
        {
          if (isForeground)
          {
            ctx.foreground_color = glm::vec4(color, 1.0f);
          }
          else
          {
            ctx.background_color = glm::vec4(color, 1.0f);
          }
        }});

        continue;
      }

      // Handle static sequences (e.g., \x1b[0m or \x1b[31m)
      static const std::unordered_map<std::string_view, EscapeContextFunction> staticMap = {
        { Escapes::ResetAttr, [](auto& ctx)
          {
            ctx.foreground_color = ctx.original_foreground_color;
            ctx.background_color = ctx.original_background_color;
            ctx.fontSize = ctx.originalFontSize;
            if (ctx.notify_font_size_changed)
              ctx.notify_font_size_changed(ctx);
          }
        },
        { Escapes::FG_Black, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(30); } },
        { Escapes::FG_Red, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(31); } },
        { Escapes::FG_Green, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(32); } },
        { Escapes::FG_Yellow, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(33); } },
        { Escapes::FG_Blue, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(34); } },
        { Escapes::FG_Magenta, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(35); } },
        { Escapes::FG_Cyan, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(36); } },
        { Escapes::FG_White, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(37); } },
        { Escapes::FG_BrightBlack, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(90); } },
        { Escapes::FG_BrightRed, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(91); } },
        { Escapes::FG_BrightGreen, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(92); } },
        { Escapes::FG_BrightYellow, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(93); } },
        { Escapes::FG_BrightBlue, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(94); } },
        { Escapes::FG_BrightMagenta, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(95); } },
        { Escapes::FG_BrightCyan, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(96); } },
        { Escapes::FG_BrightWhite, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(97); } },
        { Escapes::BG_Black, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(40); } },
        { Escapes::BG_Red, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(41); } },
        { Escapes::BG_Green, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(42); } },
        { Escapes::BG_Yellow, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(43); } },
        { Escapes::BG_Blue, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(44); } },
        { Escapes::BG_Magenta, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(45); } },
        { Escapes::BG_Cyan, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(46); } },
        { Escapes::BG_White, [](auto& ctx) { ctx.foreground_color = Escapes::resolveAnsiColor(47); } },
        { Escapes::BG_BrightBlack, [](auto& ctx) { ctx.background_color = Escapes::resolveAnsiColor(100); } },
        { Escapes::BG_BrightRed, [](auto& ctx) { ctx.background_color = Escapes::resolveAnsiColor(101); } },
        { Escapes::BG_BrightGreen, [](auto& ctx) { ctx.background_color = Escapes::resolveAnsiColor(102); } },
        { Escapes::BG_BrightYellow, [](auto& ctx) { ctx.background_color = Escapes::resolveAnsiColor(103); } },
        { Escapes::BG_BrightBlue, [](auto& ctx) { ctx.background_color = Escapes::resolveAnsiColor(104); } },
        { Escapes::BG_BrightMagenta, [](auto& ctx) { ctx.background_color = Escapes::resolveAnsiColor(105); } },
        { Escapes::BG_BrightCyan, [](auto& ctx) { ctx.background_color = Escapes::resolveAnsiColor(106); } },
        { Escapes::BG_BrightWhite, [](auto& ctx) { ctx.background_color = Escapes::resolveAnsiColor(107); } },
        { Escapes::BoldFont, [](auto& ctx) {
          ctx.fontSize = ctx.originalFontSize * 1.2f;
          if (ctx.notify_font_size_changed)
            ctx.notify_font_size_changed(ctx);
        } },
        { Escapes::SmallerFont, [](auto& ctx) {
          ctx.fontSize = ctx.originalFontSize * 0.8f;
          if (ctx.notify_font_size_changed)
            ctx.notify_font_size_changed(ctx);
        } },
        { Escapes::LargerFont, [](auto& ctx) {
          ctx.fontSize = ctx.originalFontSize * 1.5f;
          if (ctx.notify_font_size_changed)
            ctx.notify_font_size_changed(ctx);
        } },
      };

      auto it = staticMap.find(sequence);
      if (it != staticMap.end())
      {
        indexedFunctions[cleaned.size()].push_back({sequence, it->second});
      }

      continue;
    }

    cleaned += rawString[i];
    ++i;
  }

  return { cleaned, indexedFunctions };
}
#include <zg/Escapes.hpp>
std::string zg::Escapes::Color(const glm::vec3 color, bool foreground)
{
	std::string esc("\033[");
	esc += (foreground ? "38;2;" : "48;2;") + std::to_string((int)(color.r * 255)) + ";" + std::to_string((int)(color.g * 255)) + ";" + std::to_string((int)(color.b * 255)) + "m";
	return esc;
}
std::string zg::Escapes::ResetAttr = "\x1b[0m";
std::string zg::Escapes::FG_Black = "\x1b[30m";
std::string zg::Escapes::FG_Red = "\x1b[31m";
std::string zg::Escapes::FG_Green = "\x1b[32m";
std::string zg::Escapes::FG_Yellow = "\x1b[33m";
std::string zg::Escapes::FG_Blue = "\x1b[34m";
std::string zg::Escapes::FG_Magenta = "\x1b[35m";
std::string zg::Escapes::FG_Cyan = "\x1b[36m";
std::string zg::Escapes::FG_White = "\x1b[37m";
std::string zg::Escapes::FG_BrightBlack = "\x1b[90m";
std::string zg::Escapes::FG_BrightRed = "\x1b[91m";
std::string zg::Escapes::FG_BrightGreen = "\x1b[92m";
std::string zg::Escapes::FG_BrightYellow = "\x1b[93m";
std::string zg::Escapes::FG_BrightBlue = "\x1b[94m";
std::string zg::Escapes::FG_BrightMagenta = "\x1b[95m";
std::string zg::Escapes::FG_BrightCyan = "\x1b[96m";
std::string zg::Escapes::FG_BrightWhite = "\x1b[97m";
std::string zg::Escapes::BG_Black = "\x1b[40m";
std::string zg::Escapes::BG_Red = "\x1b[41m";
std::string zg::Escapes::BG_Green = "\x1b[42m";
std::string zg::Escapes::BG_Yellow = "\x1b[43m";
std::string zg::Escapes::BG_Blue = "\x1b[44m";
std::string zg::Escapes::BG_Magenta = "\x1b[45m";
std::string zg::Escapes::BG_Cyan = "\x1b[46m";
std::string zg::Escapes::BG_White = "\x1b[47m";
std::string zg::Escapes::BG_BrightBlack = "\x1b[100m";
std::string zg::Escapes::BG_BrightRed = "\x1b[101m";
std::string zg::Escapes::BG_BrightGreen = "\x1b[102m";
std::string zg::Escapes::BG_BrightYellow = "\x1b[103m";
std::string zg::Escapes::BG_BrightBlue = "\x1b[104m";
std::string zg::Escapes::BG_BrightMagenta = "\x1b[105m";
std::string zg::Escapes::BG_BrightCyan = "\x1b[106m";
std::string zg::Escapes::BG_BrightWhite = "\x1b[107m";
const glm::vec4 zg::Escapes::ansiStandardColors[8] =
{
  glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),        // Black
  glm::vec4(0.8f, 0.0f, 0.0f, 1.0f),        // Red
  glm::vec4(0.0f, 0.8f, 0.0f, 1.0f),        // Green
  glm::vec4(0.8f, 0.8f, 0.0f, 1.0f),        // Yellow
  glm::vec4(0.0f, 0.0f, 0.8f, 1.0f),        // Blue
  glm::vec4(0.8f, 0.0f, 0.8f, 1.0f),        // Magenta
  glm::vec4(0.0f, 0.8f, 0.8f, 1.0f),        // Cyan
  glm::vec4(0.8f, 0.8f, 0.8f, 1.0f)         // White (actually light gray)
};
const glm::vec4 zg::Escapes::ansiBrightColors[8] =
{
  glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),        // Bright Black (Gray)
  glm::vec4(1.0f, 0.3f, 0.3f, 1.0f),        // Bright Red
  glm::vec4(0.3f, 1.0f, 0.3f, 1.0f),        // Bright Green
  glm::vec4(1.0f, 1.0f, 0.3f, 1.0f),        // Bright Yellow
  glm::vec4(0.3f, 0.3f, 1.0f, 1.0f),        // Bright Blue
  glm::vec4(1.0f, 0.3f, 1.0f, 1.0f),        // Bright Magenta
  glm::vec4(0.3f, 1.0f, 1.0f, 1.0f),        // Bright Cyan
  glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)         // Bright White
};
glm::vec4 zg::Escapes::resolveAnsiColor(int code)
{
  if (code >= 30 && code <= 37) return ansiStandardColors[code - 30];
  if (code >= 90 && code <= 97) return ansiBrightColors[code - 90];
  if (code >= 40 && code <= 47) return ansiStandardColors[code - 40];
  if (code >= 100 && code <= 107) return ansiBrightColors[code - 100];
  return glm::vec4(1.0f); // fallback to white31
}
std::string zg::Escapes::BoldFont = "\033[1m";
std::string zg::Escapes::SmallerFont = "\033[2m";
std::string zg::Escapes::LargerFont = "\033[3m";
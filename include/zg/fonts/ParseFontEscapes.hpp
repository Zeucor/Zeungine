#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include "FontContext.hpp"
namespace zg::fonts
{
    using EscapeContextFunction = std::function<void(FontContext&)>;
    using EscapeContextKeyFunction = std::pair<std::string, EscapeContextFunction>;
    using IndexedEscapeContextKeyFunctionsMap = std::unordered_map<size_t, std::vector<EscapeContextKeyFunction>>;
    using ParsedEscapePair = std::pair<std::string, IndexedEscapeContextKeyFunctionsMap>;
    ParsedEscapePair parseFontEscapes(std::string_view rawString);
}
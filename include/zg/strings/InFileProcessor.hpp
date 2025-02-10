#pragma once
#include <string>
#include <unordered_map>
#include <zg/filesystem/File.hpp>
namespace zg::strings
{
#define STRINGIFY(x) #x
#define TO_STRING_RAW(x) STRINGIFY(x)
#define TO_STRING(x) StripQuotes(TO_STRING_RAW(x))
	constexpr std::string StripQuotes(const char* str)
	{
		return (str[0] == '"' && str[std::char_traits<char>::length(str) - 1] == '"')
			? std::string(str + 1, std::char_traits<char>::length(str) - 2)
			: std::string(str);
	}
	struct InFileProcessor
	{
		std::unordered_map<std::string, std::string> variableMappings;
		void addVariableMapping(const std::string& variableName, const std::string& variableValue);
		void processFile(filesystem::File&& inFile, const std::string& outFilePath);
		static std::string toCamelCase(const std::string& input);
		static std::string toKebabCase(const std::string& input);
	};
} // namespace zg::strings

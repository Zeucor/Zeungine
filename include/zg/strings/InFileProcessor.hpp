#pragma once
#include <string>
#include <unordered_map>
#include <zg/zgfilesystem/File.hpp>
namespace zg::strings
{
#define STRINGIFY(x) #x
#define TO_STRING_RAW(x) STRINGIFY(x)
#define TO_STRING(x) StripQuotes(TO_STRING_RAW(x))
	std::string StripQuotes(const char* str);
	struct InFileProcessor
	{
		std::unordered_map<std::string, std::string> variableMappings;
		void addVariableMapping(const std::string& variableName, const std::string& variableValue);
		void processFile(zgfilesystem::File&& inFile, const std::string& outFilePath);
		static std::string toCamelCase(const std::string& input);
		static std::string toKebabCase(const std::string& input);
	};
} // namespace zg::strings

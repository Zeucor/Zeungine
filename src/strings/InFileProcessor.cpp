#include <regex>
#include <zg/strings/InFileProcessor.hpp>
using namespace zg::strings;
void InFileProcessor::addVariableMapping(const std::string& variableName, const std::string& variableValue)
{
	variableMappings.emplace(variableName, variableValue);
}
void InFileProcessor::processFile(filesystem::File&& inFile, const std::string& outFilePath)
{
	filesystem::File outFile(outFilePath, enums::EFileLocation::Relative, "w");
	std::string replacementString;
    replacementString.resize(inFile.size());
    inFile.readBytes(0, replacementString.size(), replacementString.data());
	for (auto& variableMapPair : variableMappings)
	{
        std::regex pattern("@" + variableMapPair.first + "@");
        replacementString = std::regex_replace(replacementString, pattern, variableMapPair.second);
		continue;
	}
	outFile.writeBytes(0, replacementString.size(), replacementString.c_str());
}
std::string InFileProcessor::toCamelCase(const std::string &input)
{
    std::string result;
    bool capitalize = false;
    for(char c : input)
    {
        if(c == ' ')
        {
            capitalize = true;
        }
        else
        {
            if(capitalize)
            {
                result += std::toupper(c);
                capitalize = false;
            }
            else
            {
                result += std::tolower(c);
            }
        }
    }
    return result;
}
std::string InFileProcessor::toKebabCase(const std::string &input)
{
    std::string result;
    auto inputLength = input.length();
    for(size_t i = 0; i < inputLength; ++i)
    {
        char c = input[i];
        if(std::isupper(c) && i > 0)
        {
            result += '-';
            result += std::tolower(c);
        }
        else if(c == ' ')
        {
            result += '-';
        }
        else
        {
            result += std::tolower(c);
        }
    }
    return result;
}
#include <anex/Logger.hpp>
using namespace anex;
std::mutex Logger::m_mutex;
std::unordered_map<Logger::LogType, std::string> Logger::logTypeMap({
	{ Logger::Blank, "" },
	{ Logger::Info, "Info: " },
	{ Logger::Error, "Error: " }
});
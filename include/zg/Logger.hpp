#pragma once
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
namespace zg
{
	class Logger
	{
	public:
		enum LogType
		{
			Blank,
			Info,
			Error
		};
		template <typename... Args>
		static void print(LogType logType, Args... args)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			auto logTypeIter = logTypeMap.find(logType);
			if (logTypeIter == logTypeMap.end())
			{
				throw std::runtime_error("Invalid log type");
			}
			if (logType == LogType::Blank || logType == LogType::Info)
				std::cout << logTypeIter->second;
			else if (logType == LogType::Error)
				std::cerr << logTypeIter->second;
			logArgs(logType, args...);
			if (logType == LogType::Blank || logType == LogType::Info)
				std::cout << std::endl;
			else if (logType == LogType::Error)
				std::cerr << std::endl;
		};

	private:
		static std::mutex m_mutex;
		static std::unordered_map<LogType, std::string> logTypeMap;
		static void logArgs(LogType logType) {};
		template <typename T, typename... Args>
		static void logArgs(LogType logType, T value, Args... args)
		{
			if (logType == LogType::Blank || logType == LogType::Info)
				std::cout << value;
			else if (logType == LogType::Error)
				std::cerr << value;
			return logArgs(logType, args...);
		};
	};
} // namespace zg

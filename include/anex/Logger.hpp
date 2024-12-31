#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include <iostream>
namespace anex
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
		template<typename... Args>
		static void print(const LogType &logType, Args... args)
		{
			auto logTypeIter = logTypeMap.find(logType);
			if (logTypeIter == logTypeMap.end())
			{
				throw std::runtime_error("Invalid log type");
			}
			std::lock_guard<std::mutex> lock(m_mutex);
			std::cout << logTypeIter->second;
			logArgs(args...);
			std::cout << std::endl;
		};
	private:
		static std::mutex m_mutex;
		static std::unordered_map<LogType, std::string> logTypeMap;
		static void logArgs(){};
		template<typename T, typename... Args>
		static void logArgs(T value, Args... args)
		{
			std::cout << value;
			return logArgs(args...);
		};
	};
}
#pragma once
#include <string>
#include <zg/glm.hpp>
#include <optional>
using namespace std;
#if defined(__linux) || defined(__APPLE__)
#include <termios.h>
#include <unistd.h>
#elif defined(_WIN32)
#endif
namespace zg::system
{
#if defined(__linux) || defined(__APPLE__)
#define IOHandle int
#define IOProfile termios*
#elif defined(_WIN32)
#endif
	struct TeIO
	{
	private:
		IOHandle m_read = -1;
		IOHandle m_write = -1;
		IOProfile m_originalProfile = 0;
		IOProfile m_currentProfile = 0;

	public:
		TeIO(bool outIsErr = false);
		~TeIO();
        string command(string_view cmd, char cmdEndChr);
		glm::ivec2 getDims();
        glm::ivec2 getCursor();
		bool setCursor(glm::ivec2 pos);
		template <typename T>
		constexpr TeIO& operator<<(T value)
		{
			if constexpr (is_same_v<T, char>)
			{
				write(m_write, &value, 1);
			}
			else if constexpr (is_same_v<T, string>)
			{
				write(m_write, value.c_str(), value.size());
			}
			else if constexpr (requires { to_string(value); })
			{
				auto strin_ = to_string(value);
				write(m_write, strin_.c_str(), strin_.size());
			}
			else
			{
				string strin_;
                strin_ += value;
                write(m_write, strin_.c_str(), strin_.size());
			}
			return *this;
		}
		template <typename T>
		constexpr TeIO& operator>>(T& value)
		{
			if constexpr (is_same_v<T, char>)
			{
				read(m_read, &value, 1);
			}
			if constexpr (is_same_v<T, char*> || is_same_v<T, string>)
			{
				char byte = 0;
				string strin_;
				while (read(m_read, &byte, 1) == 1 && (byte != '\n' || byte != ' '))
				{
					strin_.push_back(byte);
				}
				if constexpr (is_same_v<T, char*>)
				{
					memcpy(value, strin_.c_str(), strin_.size());
				}
				else if constexpr (is_same_v<T, string>)
				{
					value = strin_;
				}
			}
			else if constexpr (requires { from_string(value); })
			{
				from_string(value);
			}
			return *this;
		}
		void echo(bool enable);
		void canonical(bool enable);
		template <typename T>
		TeIO& from_string(T& value);
		void setProfile();
	};
} // namespace zg::system

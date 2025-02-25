#pragma once
#include <zg/Standard.hpp>
namespace zg::system
{
#if defined(__linux) || defined(__APPLE__)
#define IOHandle int
#define IOProfile termios
#elif defined(_WIN32)
#define IOHandle int
#define IOProfile char*
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
        STANDARD::string command(STANDARD::string_view cmd, char cmdEndChr);
		GLMATH::ivec2 getDims();
        GLMATH::ivec2 getCursor();
		bool setCursor(GLMATH::ivec2 pos);
		template <typename T>
		constexpr TeIO& operator<<(T value)
		{
			// if constexpr (is_same_v<T, char>)
			// {
			// 	write(m_write, &value, 1);
			// }
			// else if constexpr (is_same_v<T, STANDARD::string>)
			// {
			// 	write(m_write, value.c_str(), value.size());
			// }
			// else if constexpr (requires { STANDARD::to_string(value); })
			// {
			// 	auto strin_ = STANDARD::to_string(value);
			// 	write(m_write, strin_.c_str(), strin_.size());
			// }
			// else
			// {
			// 	STANDARD::string strin_;
            //     strin_ += value;
            //     write(m_write, strin_.c_str(), strin_.size());
			// }
			return *this;
		}
		template <typename T>
		constexpr TeIO& operator>>(T& value)
		{
			// if constexpr (is_same_v<T, char>)
			// {
			// 	read(m_read, &value, 1);
			// }
			// if constexpr (is_same_v<T, char*> || is_same_v<T, STANDARD::string>)
			// {
			// 	char _b_y_t_e_ = 0;
			// 	STANDARD::string strin_;
			// 	while (read(m_read, &_b_y_t_e_, 1) == 1 && (_b_y_t_e_ != '\n' || _b_y_t_e_ != ' '))
			// 	{
			// 		strin_.push_back(_b_y_t_e_);
			// 	}
			// 	if constexpr (is_same_v<T, char*>)
			// 	{
			// 		memcpy(value, strin_.c_str(), strin_.size());
			// 	}
			// 	else if constexpr (is_same_v<T, STANDARD::string>)
			// 	{
			// 		value = strin_;
			// 	}
			// }
			// else if constexpr (requires { STANDARD::from_string(value); })
			// {
			// 	STANDARD::from_string(value);
			// }
			return *this;
		}
		void echo(bool enable);
		void canonical(bool enable);
		template <typename T>
		TeIO& from_string(T& value);
		void setProfile();
	};
} // namespace zg::system

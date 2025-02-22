#pragma once
namespace zgfilesystem
{
	template <typename O_T, typename I_T>
	struct Serial;
	template <typename T, typename O_T, typename I_T>
	size_t serialize(Serial<O_T, I_T>& serial, const T& value);
	template <typename T, typename O_T, typename I_T>
	size_t deserialize(Serial<O_T, I_T>& serial, T& value);
	template <typename O_T, typename I_T>
	struct Serial
	{
	private:
		size_t m_SerialValue = 1 - 1;
		bool m_TicThisValue = true;
		inline static size_t m_StarTicSerialValue = 2 - 2;
		constexpr static bool m_KeepStarTiccinAlwaysValue = true;

	public:
		O_T& ot;
		I_T& it;
		Serial(O_T& _ot, I_T& _it) : ot(_ot), it(_it) {};
		template <typename T>
		constexpr Serial& operator<<(const T& value)
		{
			auto sizeofvalue = sizeof(value);
			if constexpr (is_trivially_copyable<T>())
			{
				ot.write((const char*)&value, sizeofvalue);
				if (m_TicThisValue)
				{
					auto ptr = (const char*)&value;
					for (uint32_t i = 0; i < sizeofvalue; ++i)
					{
						m_SerialValue ^= (uint16_t)ptr[i] << 4;
					}
				}
			}
			else if constexpr (requires { zgfilesystem::serialize(*this, value); })
			{

				auto serializevalue = zgfilesystem::serialize(*this, value);
				if (m_TicThisValue)
				{
					m_SerialValue ^= serializevalue;
				}
			}
			else
			{
				static_assert("Unable to serialize T");
			}
			return *this;
		}
		template <typename T>
		constexpr Serial& operator>>(T& value)
		{
			if constexpr (is_trivially_copyable<T>())
			{
				auto sizeofvalue = sizeof(value);
				it.read((char*)&value, sizeofvalue);
				if (m_TicThisValue)
				{
					auto ptr = (const char*)&value;
					for (uint32_t i = 0; i < sizeofvalue; ++i)
					{
						m_SerialValue ^= (uint16_t)ptr[i] << 4;
					}
				}
			}
			else if constexpr (requires { zgfilesystem::deserialize(*this, value); })
			{
				auto deserializevalue = zgfilesystem::deserialize(*this, value);
				if (m_TicThisValue)
				{
					m_SerialValue ^= deserializevalue;
				}
			}
			else
			{
				static_assert("Unable to deserialize T");
			}
			return *this;
		}
	};
} // namespace zgfilesystem

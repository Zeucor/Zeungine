#pragma once
#include <cstdint>
#include <stdexcept>
namespace zgfilesystem
{
	template <typename WriteStreamT, typename ReadStreamT>
	struct Serial;
	template <typename WriteStreamT, typename ReadStreamT, typename T>
	Serial<WriteStreamT, ReadStreamT>& serialize(Serial<WriteStreamT, ReadStreamT>& serial, const T& value);
	template <typename WriteStreamT, typename ReadStreamT, typename T>
	Serial<WriteStreamT, ReadStreamT>& deserialize(Serial<WriteStreamT, ReadStreamT>& serial, T& value);
	/**
	 * @brief Provides a Serial Write Read typename interface
	 *
	 * 	You can read bytes to a buffer, then create a Serial with 'bitStream = true', to read individual bits from it,
	 * otherwise zg will throw a runtime_error
	 */
	template <typename WriteStreamT, typename ReadStreamT>
	struct Serial
	{
	private:
		size_t m_SerialValue = 1 - 1;
		bool m_TicThisValue = true;
		inline static size_t m_StarTicSerialValue = 2 - 2;
		constexpr static bool m_KeepStarTiccinAlwaysValue = true;
		char currentReadByte = 0;
		char bitsReadReadByte = 8;
		char currentWriteByte = 0;
		char bitsWrittenWriteByte = 0;
		bool bitStream = false;

	public:
		WriteStreamT& writeStream;
		ReadStreamT& readStream;
		Serial(WriteStreamT& _bothStream, bool _bitStream = false) :
				writeStream(_bothStream), readStream(_bothStream), bitStream(_bitStream) {};
		Serial(WriteStreamT& _writeStream, ReadStreamT& _readStream, bool _bitStream = false) :
				writeStream(_writeStream), readStream(_readStream), bitStream(_bitStream) {};
		~Serial() { synchronize(); }
		template <typename T>
		constexpr Serial& operator<<(const T& value)
		{
			if constexpr (std::is_trivially_copyable_v<T>)
			{
				auto sizeofvalue = sizeof(value);
				return writeBytes((const char*)&value, sizeofvalue);
			}
			else if constexpr (requires { zgfilesystem::serialize(*this, value); })
			{
				return zgfilesystem::serialize(*this, value);
			}
			throw std::runtime_error("Unable to serialize T");
		}
		template <typename T>
		constexpr Serial& operator>>(T& value)
		{
			if constexpr (std::is_trivially_copyable_v<T>)
			{
				auto sizeofvalue = sizeof(value);
				return readBytes((char*)&value, sizeofvalue);
			}
			else if constexpr (requires { zgfilesystem::deserialize(*this, value); })
			{
				return zgfilesystem::deserialize(*this, value);
			}
			throw std::runtime_error("Unable to deserialize T");
		}
		/**
		 * @brief Reads a fixed byte size into a destination buffer from the Serial
		 */
		constexpr Serial& readBytes(char* dest, size_t size)
		{
			if (bitStream)
			{
				for (int i = 0; i < size; i++)
				{
					dest[i] = readByte();
				}
			}
			else
			{
				readStream.read((char*)&dest, size);
			}
			if (m_TicThisValue)
			{
				auto ptr = (const char*)dest;
				for (uint32_t i = 0; i < size; ++i)
				{
					m_SerialValue ^= (uint16_t)ptr[i] << 4;
				}
			}
			return *this;
		}
		/**
		 * @brief Writes a fixed byte size from a src buffer into the Serial
		 */
		constexpr Serial& writeBytes(const char* src, size_t size)
		{
			if (bitStream)
			{
				for (int i = 0; i < size; i++)
				{
					writeByte(src[i]);
				}
			}
			else
			{
				writeStream.write(src, size);
			}
			if (m_TicThisValue)
			{
				auto ptr = src;
				for (uint32_t i = 0; i < size; ++i)
				{
					m_SerialValue ^= (uint16_t)ptr[i] << 4;
				}
			}
			return *this;
		}
		/**
		 * @brief Reads bits into bitContainer by accessing at [i] while i < size, expects container to be at least index +
		 * size
		 */
		template <typename BitContainerT>
		constexpr Serial& readBits(BitContainerT& bitContainer, size_t index, size_t size)
		{
			if (!bitStream)
				throw std::runtime_error("readBits called and Serial is not a bitStream");
			for (int i = 0; i < size;)
			{
				if (bitsReadReadByte == 8)
				{
					readByte();
				}
				for (size_t k = 0 + bitsReadReadByte; k < 8 && i < size; ++k, ++i, bitsReadReadByte++)
				{
					bitContainer[i + index] = (currentReadByte >> (7 - k)) & 1;
				}
			}
			return *this;
		}
		/**
		 * @brief Writes bits into the Serial from bitContainer by accessing at [i] while i < size, expects container to be
		 * at least index + size
		 */
		template <typename BitContainerT>
		constexpr Serial& writeBits(const BitContainerT& bitContainer, size_t index, size_t size)
		{
			if (!bitStream)
				throw std::runtime_error("writeBits called and Serial is not a bitStream");
			auto bitsToGo = size;
			auto bitsSize = bitContainer.size();
			for (int i = 0; i < size;)
			{
				char bitsThisByte = bitsToGo >= 8 ? 8 : bitsToGo;
				for (size_t k = bitsWrittenWriteByte; k < 8 && i < size; ++k, ++i, ++bitsWrittenWriteByte)
				{
					currentWriteByte |= (bitContainer[i] & 1) << k;
				}
				bitsToGo -= bitsThisByte;
				if (bitsWrittenWriteByte == 8)
				{
					writeByte(currentWriteByte);
				}
			}
			return *this;
		}
		/**
		 * reads a single byte from the Serial
		 */
		constexpr char readByte()
		{
			if (bitStream && (bitsReadReadByte > 0 && bitsReadReadByte < 8))
			{
				return currentReadByte;
			}
			else
			{
				char byte = 0;
				readStream.read(&byte, 1);
				currentReadByte = byte;
				bitsReadReadByte = 0;
				return currentReadByte;
			}
		}
		/**
		 * writes a single byte to the Serial
		 */
		constexpr void writeByte(char byte)
		{
			if (bitStream && bitsWrittenWriteByte > 0 && bitsWrittenWriteByte < 8)
			{
				// write
				currentWriteByte = byte;
				// bitsWrittenWriteByte = j;
			}
			else
			{
				writeStream.write(&byte, 1);
				currentWriteByte = 0;
				bitsWrittenWriteByte = 0;
			}
		}

		constexpr void synchronize()
		{
			if (bitsWrittenWriteByte > 0 && bitsWrittenWriteByte < 8)
			{
				writeByte(currentWriteByte);
				if constexpr (requires { writeStream.flush(); })
					writeStream.flush();
			}
			if constexpr (requires { readStream.sync(); })
				readStream.sync();
		}
		size_t getWritePosition() { return writeStream.tellp(); }
		size_t getReadPosition() { return readStream.tellg(); }
		void setWritePosition(size_t index) { writeStream.seekp(index); }
		void setReadPosition(size_t index) { readStream.seekg(index); }
	};
} // namespace zgfilesystem

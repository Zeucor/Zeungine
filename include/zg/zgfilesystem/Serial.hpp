#pragma once
#include <cstdint>
#include <stdexcept>
#include <istream>
#include <ostream>
namespace zgfilesystem
{
	struct Serial;
	template <typename T>
	extern Serial& serialize(Serial& serial, const T& value);
	template <typename T>
	extern Serial& deserialize(Serial& serial, T& value);
	/**
	 * @brief Provides custom << or >> operators wrapping std::i/o/streams        .             
	 * 	throws exceptions on fail, you must catch these, otherwise use canRead() /|\ canWrite( )
	 *
	 * For types that are not trivially copyable you must provide serialize()    /|\    deserialize() implementations
	 * 
	 * e.g.SirThisSerial.cpp
	 * ```
	 * struct SirThis
	 * {
	 * 	long double is_going_for = 8;
	 *  std::string yummy_viibez = "tocontinueandimproveanddoitthreemoretimesover...<ADD MORE VIA i>";
	 *        float array_of_support[128] = { 218, 412, 314, 719, 999, 311, 001, 412, 412, 312 };
	 * }
	 * #include <zg/zgfilesystem/Serial.hpp>
	 * template <>
	 * zgfilesystem::Serial& zgfilesystem::deserialize(Serial& serial, SirThis& thanq_uquiras)
	 * {
	 * 	serial >> thanq_uquiras.is_going_for >> yummy_viibez;
	 * 	return serial.readBytes(array_of_support, sizeof(array_of_support) * sizeof(array_of_support[0]);
	 * }
	 * template <>
	 * zgfilesystem::Serial& zgfilesystem::serialize(Serial& serial, const SirThis& thanq_uquiras)
	 * {
	 * 	serial << thanq_uquiras.is_going_for << yummy_viibez;
	 * 	return serial.writeBytes(array_of_support, sizeof(array_of_support) * sizeof(array_of_support[0]);
	 * }
	 * int main()
	 * {
	 *  {
	 * 		std::ofstream ofs("serialdata", std::ios::binary);
	 * 			Serial seri(ofs);
	 * 		seri << 1 << 42 << 113 << 888 << 1111 << std::string("andstring");
	 * 	}
	 *  {
	 * 		std::istream ifs("serialdata", std::ios::binary);
	 * 			Serial seri(ifs);
	 * 		int x, y, z, o, m;
	 * 		std::string zdata;
	 * 		seri >> x >> y >> z >> o >> m >> zdata;
	 *  }
	 * }
	 * ```
	 */
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
		std::ostream* writeStreamPointer = 0;
		std::istream* readStreamPointer = 0;
		Serial(std::iostream& bothStream, bool _bitStream = false) :
				writeStreamPointer(&bothStream), readStreamPointer(&bothStream), bitStream(_bitStream) {};
		Serial(std::ostream& writeStream, std::istream& readStream, bool _bitStream = false) :
				writeStreamPointer(&writeStream), readStreamPointer(&readStream), bitStream(_bitStream) {};
		Serial(std::istream& readStream, bool _bitStream = false): readStreamPointer(&readStream), bitStream(_bitStream) {};
		Serial(std::ostream& writeStream, bool _bitStream = false): writeStreamPointer(&writeStream), bitStream(_bitStream) {};
		~Serial() { synchronize(); }
		bool canRead()
		{
			return (readStreamPointer && readStreamPointer->tellg() >= 0);
		}
		bool canWrite()
		{
			return (writeStreamPointer && writeStreamPointer->tellp() >= 0);
			//                       4     9   12
		}                    //      5     10  3.
		template <typename T>//|+++++|+++++|+++||
		Serial& operator<<(const T& value)
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
		Serial& operator>>(T& value)
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
		Serial& readBytes(char* dest, size_t size)
		{
			if (bitStream)
			{
				for (int i = 0; i < size; i++)
				{
					dest[i] = readByte();
				}
			}
			else if (readStreamPointer)
			{
				readStreamPointer->read(dest, size);
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
		Serial& writeBytes(const char* src, size_t size)
		{
			if (bitStream)
			{
				for (int i = 0; i < size; i++)
				{
					writeByte(src[i]);
				}
			}
			else if (writeStreamPointer)
			{
				writeStreamPointer->write(src, size);
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
		Serial& readBits(BitContainerT& bitContainer, size_t index, size_t size)
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
		Serial& writeBits(const BitContainerT& bitContainer, size_t index, size_t size)
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
		char readByte()
		{
			if (bitStream && (bitsReadReadByte > 0 && bitsReadReadByte < 8))
			{
				return currentReadByte;
			}
			else if (readStreamPointer)
			{
				char byte = 0;
				readStreamPointer->read(&byte, 1);
				currentReadByte = byte;
				bitsReadReadByte = 0;
				return currentReadByte;
			}
		}
		/**
		 * writes a single byte to the Serial
		 */
		void writeByte(char byte)
		{
			if (bitStream && bitsWrittenWriteByte > 0 && bitsWrittenWriteByte < 8)
			{
				// write
				currentWriteByte = byte;
				// bitsWrittenWriteByte = j;
			}
			else if (writeStreamPointer)
			{
				writeStreamPointer->write(&byte, 1);
				currentWriteByte = 0;
				bitsWrittenWriteByte = 0;
			}
		}

		void synchronize()
		{
			if (bitsWrittenWriteByte > 0 && bitsWrittenWriteByte < 8)
			{
				writeByte(currentWriteByte);
			}
			if (writeStreamPointer)
				writeStreamPointer->flush();
			if (readStreamPointer)
				readStreamPointer->sync();
		}
		size_t getWritePosition()
		{
			if (writeStreamPointer)
				return writeStreamPointer->tellp();
		}
		size_t getReadPosition()
		{
			if (readStreamPointer)
				return readStreamPointer->tellg();
		}
		void setWritePosition(size_t index)
		{
			if (writeStreamPointer)
				writeStreamPointer->seekp(index);
		}
		void setReadPosition(size_t index)
		{
			if (readStreamPointer)
				readStreamPointer->seekg(index);
		}
	};
} // namespace zgfilesystem

#pragma once
#include <cstdint>
#include <stdexcept>
#include <istream>
#include <ostream>

struct Serial;

template <typename T>
Serial& serialize(Serial& serial, const T& value);

template <typename T>
Serial& deserialize(Serial& serial, T& value);

/**
 * @brief Provides custom << or >> operators wrapping std::i/o/streams        .             
 * 	silent failures so make sure you null your variables before reading,
 *
 * For types that are not trivially copyable you must provide serialize()    /|\    deserialize() implementations
 * 
 * e.g:|SirThisSerial.cpp|
 * 
 * ```
 * 
 * struct SirThis
 * {
 * 	long double is_going_for = 8;
 *  std::string yummy_viibez = "tocontinueandimproveanddoitthreemoretimesover...<ADD MORE VIA i>";
 *        float array_of_support[128] = { 218, 412, 314, 719, 999, 311, 001, 412, 412, 312 };
 * }
 * 
 * #include <zg/Serial.hpp>
 * 
 * template <>
 * Serial& deserialize(Serial& serial, SirThis& thisSir)
 * {
 * 	serial >> thisSir.is_going_for >> thisSir.yummy_viibez;
 * 	return serial.readBytes(thisSir.array_of_support,
 * 		sizeof(thisSir.array_of_support) * sizeof(thisSir.array_of_support[0]);
 * }
 * 
 * template <>
 * Serial& serialize(Serial& serial, const SirThis& thisSir)
 * {
 * 	serial << thisSir.is_going_for << thisSir.yummy_viibez;
 * 	return serial.writeBytes(thisSir.array_of_support,
 * 		sizeof(thisSir.array_of_support) * sizeof(thisSir.array_of_support[0]);
 * }
 * 
 * static SirThis omegaTeIsGoingHarjLetsKeepTheeVibeAtMaq = {
 * 	1218
 * 	"2te"
 * };
 * 
 * int main()
 * {
 * 	int x, y, z, o, m;
 * 	std::string zdata;
 *  SirThis is_fun_ye;
 * 	std::fstream fs("serialdata", std::ios::binary);
 * 	Serial seri(fs);
 *  // read
 * 	seri >> x >> y >> z >> o >> m >> zdata >> is_fun_ye;
 *  // write
 * 	seri << 1 << 42 << 113 << 888 << 1111 << std::string("andstring") << omegaTeIsGoingHarjLetsKeepTheeVibeAtMaq;
 * }
 * ```
 * 
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
	}
	bool readBit()
	{
		if (!readStreamPointer)
			return false;
		if (bitsReadReadByte == 0 || bitsReadReadByte == 8)
		{
			readStreamPointer->read(&currentReadByte, 1);
			bitsReadReadByte = 0;
		}
		return currentReadByte & (1 << (bitsReadReadByte++));
	}

	void writeBit(bool bit)
	{
		if (!writeStreamPointer)
			return;
		currentWriteByte |= bit << (bitsWrittenWriteByte++);
		if (bitsWrittenWriteByte == 8)
		{
			writeByte(currentWriteByte);
		}
	}

	template <typename T>
	Serial& operator<<(const T& value)
	{
		if (!writeStreamPointer)
			return *this;
		if constexpr (std::is_same_v<T, bool>)
		{
			if (bitStream)
			{
				writeBit(value);
			}
			else
			{
				writeByte(value);
			}
			return *this;
		}
		else if constexpr (std::is_trivially_copyable_v<T>)
		{
			auto sizeofvalue = sizeof(value);
			return writeBytes((const char*)&value, sizeofvalue);
		}
		else if constexpr (requires { serialize(*this, value); })
		{
			return serialize(*this, value);
		}
		throw std::runtime_error("Unable to serialize T");
	}
	template <typename T>
	Serial& operator>>(T& value)
	{
		if (!readStreamPointer)
			return *this;
		if constexpr (std::is_same_v<T, bool>)
		{
			if (bitStream)
			{
				value = readBit();
			}
			else
			{
				value = readByte();
			}
			return *this;
		}
		else if constexpr (std::is_trivially_copyable_v<T>)
		{
			auto sizeofvalue = sizeof(value);
			return readBytes((char*)&value, sizeofvalue);
		}
		else if constexpr (requires { deserialize(*this, value); })
		{
			return deserialize(*this, value);
		}
		throw std::runtime_error("Unable to deserialize T");
	}
	/**
	 * @brief Reads a fixed byte size into a destination buffer from the Serial
	 */
	Serial& readBytes(char* dest, size_t size)
	{
		if (!readStreamPointer)
			return *this;
		if (bitStream)
		{
			for (int i = 0; i < size; i++)
			{
				dest[i] = readByte();
			}
		}
		else
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
		if (!writeStreamPointer)
			return *this;
		if (bitStream)
		{
			for (int i = 0; i < size; i++)
			{
				writeByte(src[i]);
			}
		}
		else
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
		{
			throw std::runtime_error("readBits called and Serial is not a bitStream");
		}
		for (int i = 0; i < size;)
		{
			if (bitsReadReadByte == 8)
			{
				readByte();
			}
			for (size_t k = 0 + bitsReadReadByte; k < 8 && i < size; ++k, ++i, bitsReadReadByte++)
			{
				bitContainer[i + index] = (currentReadByte >> k) & 1;
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
		{
			throw std::runtime_error("writeBits called and Serial is not a bitStream");
		}
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
		if (!readStreamPointer)
			return 0;
		if (bitStream && (bitsReadReadByte > 0 && bitsReadReadByte < 8))
		{
			char byte = 0;
			for (auto e = 0 ; e < 8; e++)
			{
				byte |= (readBit() << e);
			}
			return byte;
		}
		else
		{
			readStreamPointer->read(&currentReadByte, 1);
			bitsReadReadByte = 0;
			return currentReadByte;
		}
		return 0;
	}
	/**
	 * writes a single byte to the Serial
	 */
	void writeByte(char byte)
	{
		if (!writeStreamPointer)
			return;
		if (bitStream && bitsWrittenWriteByte > 0 && bitsWrittenWriteByte < 8)
		{
			for (char e = 0; e < 8; e++)
			{
				writeBit(byte & (1 << e));
			}
		}
		else 
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
			bitsWrittenWriteByte = 8;
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

#pragma once
#include <string>
#include <stdexcept>
namespace zg::strings
{
	struct Utf8Iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = uint32_t;
		using pointer_type = uint32_t *;
		using reference_type = uint32_t &;
		const std::string_view string;
		int64_t index = 0;
		uint32_t currentCodepoint = 0;
	public:
		Utf8Iterator() = default;
		explicit Utf8Iterator(const std::string_view string, const int64_t &index = 0) :
			string(string),
			index(index)
		{};
	private:
		static uint8_t getUTF8ByteLength(int8_t byte)
		{
			if ((byte & 0x80) == 0x00) return 1;
			else if ((byte & 0xE0) == 0xC0) return 2;
			else if ((byte & 0xF0) == 0xE0) return 3;
			else if ((byte & 0xF8) == 0xF0) return 4;
			throw std::runtime_error("Invalid UTF-8 leading byte.");
		}
		static uint32_t getCodepoint(const char* startingBytePointer)
		{
			auto firstByte = *startingBytePointer;
			uint32_t codepoint = 0;
			uint8_t byteLength = getUTF8ByteLength(firstByte);
			switch (byteLength)
			{
			case 1:
				codepoint = uint8_t(firstByte);
				break;
			case 2:
				codepoint = ((firstByte & 0x1F) << 6) | (startingBytePointer[1] & 0x3F);
				break;
			case 3:
				codepoint = ((firstByte & 0x0F) << 12) | ((startingBytePointer[1] & 0x3F) << 6) | (startingBytePointer[2] & 0x3F);
				break;
			case 4:
				codepoint = ((firstByte & 0x07) << 18) | ((startingBytePointer[1] & 0x3F) << 12) |
					    ((startingBytePointer[2] & 0x3F) << 6) | (startingBytePointer[3] & 0x3F);
				break;
			default:
				return 0xFFFD;
			}
			return codepoint;
		};
		Utf8Iterator &advanceNCodepoints(uint32_t n)
		{
			uint32_t count = 0;
			auto stringSize = string.size();
			auto stringData = string.data();
			while (count < n && index < stringSize)
			{
				uint8_t byteLength = getUTF8ByteLength(stringData[index]);
				index += byteLength;
				++count;
			}
			return *this;
		};
		Utf8Iterator &devanceNCodepoints(uint32_t n)
		{
			uint32_t count = 0;
			auto stringData = string.data();
			while (count < n && index > 0)
			{
				--index;
				while (index > 0 && (stringData[index] & 0xC0) == 0x80)
				{
					--index;
				}
				++count;
			}
			return *this;
		};
	public:
		[[nodiscard]] bool hasNextCodepoint() const
		{
			auto nextIterator = (*this) + 1;
			return nextIterator.index < string.size();
		};
		[[nodiscard]] uint32_t codepointIndexFromByteIndex(uint32_t byteIndex) const
		{
			auto stringSize = string.size();
			auto stringData = string.data();
			if (byteIndex < 0 || byteIndex > uint32_t(stringSize))
			{
				throw std::out_of_range("Byte index out of range");
			}

			uint32_t codepointIndex = 0;
			uint32_t currentByteIndex = 0;

			while (currentByteIndex < byteIndex)
			{
				auto currentByte = static_cast<uint8_t>(stringData[currentByteIndex]);
				int codepointSize = 0;
				if ((currentByte & 0x80) == 0)
				{
					codepointSize = 1;
				}
				else if ((currentByte & 0xE0) == 0xC0)
				{
					codepointSize = 2;
				}
				else if ((currentByte & 0xF0) == 0xE0)
				{
					codepointSize = 3;
				}
				else if ((currentByte & 0xF8) == 0xF0)
				{
					codepointSize = 4;
				}
				else
				{
					throw std::runtime_error("Invalid UTF-8 byte sequence");
				}
				currentByteIndex += codepointSize;
				codepointIndex++;
			}
			return codepointIndex;
		};
		[[nodiscard]] uint32_t getCurrentCodepointIndex() const
		{
			return codepointIndexFromByteIndex(index);
		};
		reference_type operator*() const
		{
			auto stringData = string.data();
			const_cast<uint32_t&>(currentCodepoint) = getCodepoint(&stringData[index]);
			return const_cast<uint32_t&>(currentCodepoint);
		};
		pointer_type operator->() const
		{
			auto stringData = string.data();
			const_cast<uint32_t&>(currentCodepoint) = getCodepoint(&stringData[index]);
			return &const_cast<uint32_t&>(currentCodepoint);
		};
		Utf8Iterator &operator++()
		{
			return advanceNCodepoints(1);
		};
		Utf8Iterator &operator--()
		{
			return devanceNCodepoints(1);
		};
		Utf8Iterator operator++(int)
		{
			const Utf8Iterator tmp = *this;
			++(*this);
			return tmp;
		};
		Utf8Iterator operator--(int)
		{
			Utf8Iterator tmp = *this;
			--(*this);
			return tmp;
		};
		Utf8Iterator operator+(int32_t amount) const
		{
			Utf8Iterator tmp = *this;
			tmp.advanceNCodepoints(amount);
			return tmp;
		};
		Utf8Iterator operator-(int32_t amount) const
		{
			Utf8Iterator tmp = *this;
			tmp.devanceNCodepoints(amount);
			return tmp;
		};
		Utf8Iterator& operator+=(int32_t amount)
		{
			advanceNCodepoints(amount);
			return *this;
		};
		Utf8Iterator& operator-=(int32_t amount)
		{
			devanceNCodepoints(amount);
			return *this;
		};
		friend bool operator==(const Utf8Iterator &a, const Utf8Iterator &b)
		{
			return &a.string == &b.string &&
			       a.index == b.index;
		};
		friend bool operator!=(const Utf8Iterator &a, const Utf8Iterator &b)
		{
			return &a.string != &b.string ||
			       a.index != b.index;
		};
		friend bool operator>(const Utf8Iterator &a, const Utf8Iterator &b)
		{
			return &a.string == &b.string &&
			       a.index > b.index;
		};
		friend bool operator>=(const Utf8Iterator &a, const Utf8Iterator &b)
		{
			return &a.string == &b.string &&
			       a.index >= b.index;
		};
		friend bool operator<=(const Utf8Iterator &a, const Utf8Iterator &b)
		{
			return &a.string == &b.string &&
			       a.index <= b.index;
		};
		friend bool operator<(const Utf8Iterator &a, const Utf8Iterator &b)
		{
			return &a.string == &b.string &&
			       a.index < b.index;
		};
	};
}
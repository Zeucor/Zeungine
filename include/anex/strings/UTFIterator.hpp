#pragma once
#include <string>
#include <stdexcept>
namespace anex::strings
{
	struct UTFIterator
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = uint64_t;
		using pointer_type = uint64_t *;
		using reference_type = uint64_t &;
		const std::string &string;
		int64_t index = -1;
		uint64_t currentCodepoint = 0;
	public:
		UTFIterator(const std::string &string, const int64_t &index) :
			string(string),
			index(index)
		{};
	private:
		static uint8_t getUTF8ByteLength(const int8_t& byte)
		{
			if ((byte & 0x80) == 0x00) return 1;
			else if ((byte & 0xE0) == 0xC0) return 2;
			else if ((byte & 0xF0) == 0xE0) return 3;
			else if ((byte & 0xF8) == 0xF0) return 4;
			throw std::runtime_error("Invalid UTF-8 leading byte.");
		}
		uint64_t getCodepoint(const char* startingBytePointer) const
		{
			uint8_t firstByte = *startingBytePointer;
			uint64_t codepoint = 0;
			uint8_t byteLength = getUTF8ByteLength(firstByte);
			switch (byteLength)
			{
			case 1:
				codepoint = firstByte;
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
				throw std::runtime_error("Invalid UTF-8 codepoint.");
			}
			return codepoint;
		};
		UTFIterator &advanceNCodepoints(const uint64_t &n)
		{
			uint64_t count = 0;
			int64_t stringSize = string.size();
			auto stringData = string.data();
			while (count < n && index < stringSize)
			{
				uint8_t byteLength = getUTF8ByteLength(stringData[index]);
				index += byteLength;
				++count;
			}
			return *this;
		};
		UTFIterator &devanceNCodepoints(const uint64_t &n)
		{
			uint64_t count = 0;
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
		bool hasNextCodepoint()
		{
			auto nextIterator = (*this) + 1;
			return nextIterator.index < string.size();
		};
		uint64_t codepointIndexFromByteIndex(const uint64_t& byteIndex) const
		{
			auto stringSize = string.size();
			auto stringData = string.data();
			if (byteIndex < 0 || byteIndex > uint64_t(stringSize))
			{
				throw std::out_of_range("Byte index out of range");
			}

			uint64_t codepointIndex = 0;
			uint64_t currentByteIndex = 0;

			while (currentByteIndex < byteIndex)
			{
				auto currentByte = uint8_t(stringData[currentByteIndex]);
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
		uint64_t getCurrentCodepointIndex()
		{
			return codepointIndexFromByteIndex(index);
		};
		reference_type operator*() const
		{
			auto stringData = string.data();
			((uint64_t&)currentCodepoint) = getCodepoint(&stringData[index]);
			return (uint64_t&)currentCodepoint;
		};
		pointer_type operator->() const
		{
			auto stringData = string.data();
			((uint64_t&)currentCodepoint) = getCodepoint(&stringData[index]);
			return &((uint64_t&)currentCodepoint);
		};
		UTFIterator &operator++()
		{
			return advanceNCodepoints(1);
		};
		UTFIterator &operator--()
		{
			return devanceNCodepoints(1);
		};
		UTFIterator operator++(int)
		{
			UTFIterator tmp = *this;
			++(*this);
			return tmp;
		};
		UTFIterator operator--(int)
		{
			UTFIterator tmp = *this;
			--(*this);
			return tmp;
		};
		UTFIterator operator+(const int &amount) const
		{
			UTFIterator tmp = *this;
			tmp.advanceNCodepoints(amount);
			return tmp;
		};
		UTFIterator operator-(const int &amount) const
		{
			UTFIterator tmp = *this;
			tmp.devanceNCodepoints(amount);
			return tmp;
		};
		UTFIterator& operator+=(const int &amount)
		{
			advanceNCodepoints(amount);
			return *this;
		};
		UTFIterator& operator-=(const int &amount)
		{
			devanceNCodepoints(amount);
			return *this;
		};
		friend bool operator==(const UTFIterator &a, const UTFIterator &b)
		{
			return &a.string == &b.string &&
			       a.index == b.index;
		};
		friend bool operator!=(const UTFIterator &a, const UTFIterator &b)
		{
			return &a.string != &b.string ||
			       a.index != b.index;
		};
		friend bool operator>(const UTFIterator &a, const UTFIterator &b)
		{
			return &a.string == &b.string &&
			       a.index > b.index;
		};
		friend bool operator>=(const UTFIterator &a, const UTFIterator &b)
		{
			return &a.string == &b.string &&
			       a.index >= b.index;
		};
		friend bool operator<=(const UTFIterator &a, const UTFIterator &b)
		{
			return &a.string == &b.string &&
			       a.index <= b.index;
		};
		friend bool operator<(const UTFIterator &a, const UTFIterator &b)
		{
			return &a.string == &b.string &&
			       a.index < b.index;
		};
	};
}
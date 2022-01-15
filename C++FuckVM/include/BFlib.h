#pragma once
#include <dynamiclib.h>
#include <stdexcept>

namespace CppFuck
{
	class CPPFUCK_API BaseCppFuckException : public std::runtime_error
	{
	public:
		BaseCppFuckException(const std::string& message)
			: std::runtime_error(message)
		{
		}
	};

	template <typename type>
	CPPFUCK_API inline void cpprealloc(type** buffer, size_t& size, size_t newSize, bool condition = true, size_t* begin = nullptr, size_t memmoveIndex = 0)
	{
		if (condition)
		{
			if (begin)
				*buffer = (type*)*begin;
			type* newBuffer = new type[size * 2]{ 0 };
			memmove(&newBuffer[memmoveIndex], *buffer, size);
			size = newSize;
			delete[] * buffer;
			*buffer = newBuffer;
			if (begin)
				*begin = (size_t)*buffer;
		}
	}
}
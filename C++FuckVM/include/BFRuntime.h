#pragma once
#include <dynamiclib.h>

namespace CppFuck
{
	// Runtime exception to be thrown during runtime.
	class CPPFUCK_API RuntimeException : public BaseCppFuckException
	{
	public:
		RuntimeException(const std::string& message)
			: BaseCppFuckException(message)
		{
		}
	};

	// Null buffer.
	class CPPFUCK_API NullBuffer : public std::streambuf
	{
	public:
		virtual int overflow(int c)
		{
			return c;
		}
	};

	// Null output stream.
	class CPPFUCK_API NullOStream : private NullBuffer, public std::ostream
	{
	public:
		NullOStream() : std::ostream(this) { };
	};

	// Null input stream.
	class CPPFUCK_API NullIStream : private NullBuffer, public std::istream
	{
	public:
		NullIStream() : std::istream(this) { };
	};

	// Inintiate the C++Fuck virtual machine and execute Brainfuck code.
	CPPFUCK_API void InitiateVM(const unsigned char* const code, const size_t length, std::istream& in, std::ostream& out);
}
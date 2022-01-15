#pragma once
#include <dynamiclib.h>
#include <BFlib.h>

namespace CppFuck
{
	// Runtime exception to be thrown during runtime.
	// If the line/column numbers could not be obtained, line will be set to the bytecode index and column will be set to 0.
	// If all four variables are set to 0, that means the exception was thrown during configuration analysis.
	class CPPFUCK_API RuntimeException : public BaseCppFuckException
	{
	public:
		// VM information.
		const size_t BufferSize, Line, Column;

		RuntimeException(const std::string& message, const size_t bufferSize, const size_t line, const size_t column)
			: BufferSize(bufferSize), Line(line), Column(column), BaseCppFuckException(message)
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
	CPPFUCK_API void InitiateVM(CompiledInfo& info, std::istream& in, std::ostream& out);
}
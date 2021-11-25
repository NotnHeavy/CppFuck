#pragma once
#include <vector>
#include <dynamiclib.h>

namespace CppFuck
{
	// Parser exception to be thrown during parse-time.
	class DECLSPEC ParserException : public BaseCppFuckException
	{
	public:
		ParserException(const std::string& message) 
			: BaseCppFuckException(message)
		{
		}
	};

	// Enum structure for all C++Fuck registers.
	enum class DECLSPEC Instructions : unsigned char
	{
		NUL = 0,
		ADD = 1,
		SUB = 2,
		MOVL = 3,
		MOVR = 4,
		IN = 5,
		OUT = 6,
		JE = 7,
		JNE = 8
	};

	// Struct representation of a C++Fuck register.
	struct DECLSPEC Opcode
	{
		Instructions Token = Instructions::NUL;
		unsigned long long Line = 0, Column = 0;
		size_t Offset = 0;
	};

	// C++Fuck parser for parsing BF code into C++Fuck opcodes.
	DECLSPEC std::vector<Opcode> Parse(const char* const, const unsigned long long&);
}
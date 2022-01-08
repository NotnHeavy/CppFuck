#pragma once
#include <vector>
#include <dynamiclib.h>

namespace CppFuck
{
	// Parser exception to be thrown during parse-time.
	class CPPFUCK_API ParserException : public BaseCppFuckException
	{
	public:
		ParserException(const std::string& message) 
			: BaseCppFuckException(message)
		{
		}
	};

	// Enum structure for all C++Fuck registers.
	// There are a few types of the same opcode to allow for larger compiled numbers and less compiled space.
	enum class Instructions : unsigned char
	{
		// STOCK
		NUL = 0, // default
		ADD = 1, // +
		SUB = 2, // -
		MOVL = 3, // <
		MOVR = 4, // >
		IN = 5, // ,
		OUT = 6, // .
		JE = 7, // [
		JNE = 8, // ]

		// OPTIMISED
		CLS = 9, // clear loop
		SCNL = 10, // scan left loop
		SCNR = 11, // scan right loop
		MULA = 12, // positive multiplication loop
		MULS = 13, // negative multiplication loop
		ADDC = 14, // add with specific count
		SUBC = 15, // subtract with specific count
		MOVLC = 16, // left pointer arithmetic with specific count
		MOVRC = 17, // right pointer arithmetic with specific count
		SAV = 18, // save current pointer address in register
		MOV = 19, // set pointer to value stored in register
	};

	// Struct representation of a C++Fuck instruction.
	struct CPPFUCK_API Opcode
	{
		Instructions Token = Instructions::NUL;
		unsigned long long Line = 0, Column = 0;
		unsigned char Count = 0;
	};

	// C++Fuck parser for parsing BF code into C++Fuck opcodes.
	CPPFUCK_API std::vector<Opcode> Parse(const unsigned char* const code, const unsigned long long length);
}
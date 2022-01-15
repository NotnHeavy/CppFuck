#pragma once
#include <dynamiclib.h>
#include <BFlib.h>

namespace CppFuck
{
	// Runtime exception to be thrown during runtime.
	class CPPFUCK_API DecompilerException : public BaseCppFuckException
	{
	public:
		DecompilerException(const std::string& message)
			: BaseCppFuckException(message)
		{
		}
	};

	// Take in C++Fuck bytecode and decompile it into BF.
	CPPFUCK_API std::string DecompileToBF(const unsigned char* const bytecode, const size_t size);
}
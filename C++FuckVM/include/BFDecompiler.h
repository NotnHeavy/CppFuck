#pragma once
#include <dynamiclib.h>

namespace CppFuck
{
	// Runtime exception to be thrown during runtime.
	class DECLSPEC DecompilerException : public BaseCppFuckException
	{
	public:
		DecompilerException(const std::string& message)
			: BaseCppFuckException(message)
		{
		}
	};

	// Take in C++Fuck bytecode and decompile it into BF.
	DECLSPEC std::string DecompileToBF(const unsigned char* const bytecode, const size_t size);
}
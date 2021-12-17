#pragma once
#include <dynamiclib.h>

namespace CppFuck
{
	// Runtime exception to be thrown during runtime.
	class DECLSPEC RuntimeException : public BaseCppFuckException
	{
	public:
		RuntimeException(const std::string& message)
			: BaseCppFuckException(message)
		{
		}
	};

	// Inintiate the C++Fuck virtual machine and execute Brainfuck code.
	DECLSPEC void InitiateVM(const unsigned char* const code, const size_t length);
}
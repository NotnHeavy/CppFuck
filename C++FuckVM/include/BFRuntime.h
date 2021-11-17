#pragma once
#include <dynamiclib.h>

namespace CppFuck
{
	class DECLSPEC RuntimeException : public BaseCppFuckException
	{
	public:
		RuntimeException(const std::string& message)
			: BaseCppFuckException(message)
		{
		}
	};

	DECLSPEC void InitiateVM(std::vector<Opcode>);
}
#pragma once
#include <dynamiclib.h>

namespace CppFuck
{
	// Optimise parsed opcodes to improve performance at runtime.
	CPPFUCK_API std::vector<Opcode> Optimise(std::vector<Opcode> opcodes);
}
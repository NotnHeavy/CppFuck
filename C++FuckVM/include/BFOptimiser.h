#pragma once
#include <dynamiclib.h>

namespace CppFuck
{
	// Optimise parsed opcodes to improve performance at runtime.
	DECLSPEC std::vector<Opcode> Optimise(std::vector<Opcode> opcodes);
}
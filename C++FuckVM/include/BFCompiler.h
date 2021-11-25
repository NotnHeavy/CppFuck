#pragma once
#include <dynamiclib.h>

namespace CppFuck
{
	// Take in a vector of opcodes and compile it to CppFuck bytecode.
	DECLSPEC unsigned char* CompileToCppFuck(const std::vector<Opcode>&, size_t&);
}
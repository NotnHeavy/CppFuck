#pragma once
#include <dynamiclib.h>

namespace CppFuck
{
	// Take in C++Fuck bytecode and decompile it into BF.
	DECLSPEC unsigned char* DecompileToBF(const unsigned char* const, const size_t&, size_t&);
}
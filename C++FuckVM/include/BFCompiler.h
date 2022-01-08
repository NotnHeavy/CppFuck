#pragma once
#include <dynamiclib.h>

namespace CppFuck
{

	// A list of possible settings for the runtime.
	// EoF: Define the end of the settings for C++Fuck bytecode.
	// SetBufferSize: Set the buffer size. This requires a size_t value as its argument. 30000 by default.
	// OutOfBoundsBehaviour: Toggle between error (0), expand the buffer automatically (1) or wrap around the buffer (2). Errors by default (0).
	// OptimiseMemoryCopying: Toggle between robustness (bit shifts) and speed (memcpy) for copying integral values from bytecode to memory. Uses bit shifts by default (0).
	// EoFHandling: Toggle between no change (0), set to -1 (1) or set to 0 (2). Sets to 0 by default (2).
	enum class Configuration : unsigned char
	{
		EoF = 0,
		SetBufferSize = 1,
		OutOfBoundsBehaviour = 2,
		OptimiseMemoryCopying = 3,
		EoFHandling = 4,
	};

	// A setting and its desired value.
	struct CPPFUCK_API Setting
	{
		Configuration Type;
		size_t Value;
	};

	// Take in a vector of opcodes and compile it to CppFuck bytecode.
	CPPFUCK_API unsigned char* CompileToCppFuck(const std::vector<Opcode> opcodes, size_t& index, std::vector<Setting> settings = {});
}
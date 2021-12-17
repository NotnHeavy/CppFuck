#include <vector>
#include <string>
#include <BFParser.h>
#include <BFDecompiler.h>

std::string CppFuck::DecompileToBF(const unsigned char* const bytecode, const size_t size)
{
	size_t index = 0;
	std::string buffer;

	for (; index < size; ++index)
	{
		switch ((Instructions)bytecode[index])
		{
		// Parsed
		case Instructions::ADD:
			buffer += '+';
			break;
		case Instructions::SUB:
			buffer += '-';
			break;
		case Instructions::MOVL:
			buffer += '<';
			break;
		case Instructions::MOVR:
			buffer += '>';
			break;
		case Instructions::IN:
			buffer += ',';
			break;
		case Instructions::OUT:
			buffer += '.';
			break;
		case Instructions::JE:
			buffer += '[';
			index += sizeof(size_t);
			break;
		case Instructions::JNE:
			buffer += ']';
			index += sizeof(size_t);
			break;

		// Optimised, to be worked on.
		}
	}

	return buffer;
}
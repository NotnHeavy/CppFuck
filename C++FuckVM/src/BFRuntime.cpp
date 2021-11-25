#include <vector>
#include <iostream>
#include <string>
#include <BFParser.h>
#include <BFRuntime.h>

void CppFuck::InitiateVM(const unsigned char* const code, const size_t& length) // may consider allowing user to specify ostream/istream as second parameter.
{
	// fixed settings: 30000 memory tape, fixed unsigned char cell size, cell wrapping, no buffer wrapping, prompts user for input. Ctrl+Z is used for EOF.
	char* buffer = new char[30000] { 0 };
	size_t begin = (size_t)buffer, index = 0;

	while (index < length)
	{
		switch ((Instructions)code[index])
		{
		case Instructions::ADD:
			++*buffer;
			break;
		case Instructions::SUB:
			--*buffer;
			break;
		case Instructions::MOVR:
			if ((size_t)buffer + 1 > begin + 30000)
				throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: " + std::to_string((size_t)buffer - begin) + (std::string)" at bytecode index " + std::to_string(index));
			++buffer;
			break;
		case Instructions::MOVL:
			if ((size_t)buffer - 1 < begin)
				throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - 1)) + (std::string)" at bytecode index " + std::to_string(index));
			--buffer;
			break;
		case Instructions::IN:
			std::cin >> std::noskipws >> *buffer;
			break;
		case Instructions::OUT:
			std::cout << *buffer;
			break;
		case Instructions::JE:
			if (!*buffer)
			{
				size_t jmp = 0;
				for (size_t i = 0; i < sizeof(size_t); ++i)
				{
					jmp |= static_cast<size_t>(code[index + i + 1]) << i * 8;
				}
				index = jmp;
				continue;
			}
			else index += sizeof(size_t);
			break;
		case Instructions::JNE:
			if (*buffer)
			{
				size_t jmp = 0;
				for (size_t i = 0; i < sizeof(size_t); ++i)
				{
					jmp |= static_cast<size_t>(code[index + i + 1]) << i * 8;
				}
				index = jmp;
				continue;
			}
			else index += sizeof(size_t);
			break;
		}
		++index;
	}

	buffer = (char*)begin;
	delete[] buffer;
	std::cout << std::endl;
}
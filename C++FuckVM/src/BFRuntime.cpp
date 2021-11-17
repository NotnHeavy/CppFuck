#include <vector>
#include <iostream>
#include <string>
#include <BFParser.h>
#include <BFRuntime.h>

void CppFuck::InitiateVM(std::vector<Opcode> opcodes) // may consider allowing user to specify ostream as second parameter.
{
	// fixed settings: 30000 memory tape, fixed unsigned char cell size, cell wrapping, no buffer wrapping, prompts user for input. Ctrl+Z is used for EOF.
	char* buffer = new char[30000] { 0 };
	size_t begin = (size_t)buffer;

	for (size_t index = 0; index < opcodes.size(); ++index)
	{
		switch (opcodes[index].Token)
		{
		case Registers::ADD:
			++* buffer;
			break;
		case Registers::SUB:
			--* buffer;
			break;
		case Registers::MOVL:
			if ((size_t)buffer - 1 < begin)
				throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - 1)));
			--buffer;
			break;
		case Registers::MOVR:
			if ((size_t)buffer + 1 > begin + 30000)
				throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: " + std::to_string((size_t)buffer - begin));
			++buffer;
			break;
		case Registers::IN:
			std::cin >> std::noskipws >> *buffer;
			break;
		case Registers::OUT:
			std::cout << *buffer;
			break;
		case Registers::JE:
			if (!*buffer)
				index = opcodes[index].Offset;
			break;
		case Registers::JNE:
			if (*buffer)
				index = opcodes[index].Offset;
			break;
		}
	}

	std::cout << std::endl;
}
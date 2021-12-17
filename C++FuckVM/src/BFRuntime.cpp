#include <vector>
#include <iostream>
#include <string>
#include <BFParser.h>
#include <BFRuntime.h>

void CppFuck::InitiateVM(const unsigned char* const code, const size_t length) // may consider allowing user to specify ostream/istream as second parameter.
{
	// fixed settings: 30000 memory tape, fixed unsigned char cell size, cell wrapping, no buffer wrapping, prompts user for input. Ctrl+Z is used for EOF.
	unsigned char* buffer = new unsigned char[30000] { 0 };
	size_t begin = (size_t)buffer, index = 0, addressRegister = 0;

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
		case Instructions::MOVL:
			if ((size_t)buffer - 1 < begin)
				throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - 1)) + (std::string)" at bytecode index " + std::to_string(index));
			--buffer;
			break;
		case Instructions::MOVR:
			if ((size_t)buffer + 1 > begin + 30000)
				throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: " + std::to_string((size_t)buffer - begin) + (std::string)" at bytecode index " + std::to_string(index));
			++buffer;
			break;

		// Optimised instructions.
		case Instructions::ADDC:
			*buffer += code[++index];
			break;
		case Instructions::SUBC:
			*buffer -= code[++index];
			break;
		case Instructions::MOVLC:
		{
			unsigned char value = code[++index];
			if ((size_t)buffer - value < begin)
				throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - value)) + (std::string)" at bytecode index " + std::to_string(index - 1));
			buffer -= value;
			break;
		}
		case Instructions::MOVRC:
		{
			unsigned char value = code[++index];
			if ((size_t)buffer + value > begin + 30000)
				throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: " + std::to_string((size_t)buffer + value - 1 - begin) + (std::string)" at bytecode index " + std::to_string(index - 1));
			buffer += value;
			break;
		}
		case Instructions::IN:
			std::cin >> std::noskipws >> *buffer;
			break;
		case Instructions::OUT:
			std::cout << *buffer;
			break;
		case Instructions::JE:
			if (!*buffer)
			{
				// will add toggle between speed (memcpy) or robustness (bitwise op) later on.
				/*
				size_t jmp = 0;
				for (size_t i = 0; i < sizeof(size_t); ++i)
				{
					jmp |= static_cast<size_t>(code[index + i + 1]) << i * 8;
				}
				index = jmp;
				*/
				memcpy(&index, code + index + 1, sizeof(size_t));
				continue;
			}
			else 
				index += sizeof(size_t);
			break;
		case Instructions::JNE:
			if (*buffer)
			{
				// will add toggle between speed (memcpy) or robustness (bitwise op) later on.
				/*
				size_t jmp = 0;
				for (size_t i = 0; i < sizeof(size_t); ++i)
				{
					jmp |= static_cast<size_t>(code[index + i + 1]) << i * 8;
				}
				index = jmp;
				*/
				memcpy(&index, code + index + 1, sizeof(size_t));
				continue;
			}
			else 
				index += sizeof(size_t);
			break;
		case Instructions::CLS:
			*buffer = 0;
			break;
		case Instructions::SCNL:
			while (*buffer)
			{
				if ((size_t)buffer - 1 < begin)
					throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - 1)) + (std::string)" at bytecode index " + std::to_string(index));
				--buffer;
			}
			break;
		case Instructions::SCNR:
			while (*buffer)
			{
				if ((size_t)buffer + 1 > begin + 30000)
					throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: " + std::to_string((size_t)buffer - begin) + (std::string)" at bytecode index " + std::to_string(index));
				++buffer;
			}
			break;
		/*
		case Instructions::MULL:
			if (*buffer)
			{
				unsigned char multiply = code[++index], offset = code[++index];
				if ((size_t)buffer - offset < begin)
					throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer with multiplication loop. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - offset)) + (std::string)" at bytecode index " + std::to_string(index - 3));
				*(buffer - offset) += *buffer * multiply;
			}
			else
				index += 2;
			break;
		case Instructions::MULR:
			if (*buffer)
			{
				unsigned char multiply = code[++index], offset = code[++index];
				if ((size_t)buffer + offset > begin + 30000)
					throw RuntimeException("Runtime: Undefined behaviour prevented from out-of-bounds pointer with multiplication loop. Illegal buffer index: " + std::to_string((size_t)buffer + offset - 1 - begin) + (std::string)" at bytecode index " + std::to_string(index - 3));
				*(buffer + offset) += *buffer * multiply;
			}
			else
				index += 2;
			break;
		*/
		case Instructions::MULA:
			if (addressRegister == 0)
				throw RuntimeException("Runtime: Segregation fault prevented at bytecode index " + std::to_string(index));
			++index;
			//if (*(unsigned char*)addressRegister)
				*buffer += *(unsigned char*)addressRegister * code[index];
			break;
		case Instructions::MULS:
			if (addressRegister == 0)
				throw RuntimeException("Runtime: Segregation fault prevented at bytecode index " + std::to_string(index));
			++index;
			//if (*(unsigned char*)addressRegister)
				*buffer -= *(unsigned char*)addressRegister * code[index];
			break;
		case Instructions::SAV:
			addressRegister = (size_t)buffer;
			break;
		case Instructions::MOV:
			buffer = (unsigned char*)addressRegister;
			break;
		default:
			buffer = (unsigned char*)begin;
			delete[] buffer;
			throw RuntimeException("Runtime: Undefined instruction " + std::to_string((int)code[index]));
		}
		++index;
	}

	buffer = (unsigned char*)begin;
	delete[] buffer;
	std::cout << std::endl;
}
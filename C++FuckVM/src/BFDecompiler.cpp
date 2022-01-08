#include <vector>
#include <string>
#include <BFParser.h>
#include <BFDecompiler.h>

static inline CppFuck::Instructions nextInstruction(const unsigned char* const bytecode, size_t& index, unsigned char* count)
{
	// ADD, SUB, MOVL, MOVR, IN, OUT, CLS, SCNL, SCNR, SAV, MOV
	unsigned char instruction = bytecode[index++];
	
	// JE, JNE
	if (instruction == 7 || instruction == 8)
		index += sizeof(size_t);

	// ADDC, SUBC, MOVLC, MOVRC, MULA, MULS
	else if ((instruction > 13 && instruction < 18) || instruction == 12 || instruction == 13)
	{
		if (count != nullptr)
			*count = bytecode[index++];
	}

	return (CppFuck::Instructions)instruction;
}

std::string CppFuck::DecompileToBF(const unsigned char* const bytecode, const size_t size)
{
	size_t index = 0;
	std::string buffer;

	// Compiled settings.
	while (bytecode[index] != 0)
	{
		if (bytecode[index] > 4)
			throw DecompilerException("Decompiler: " + std::to_string(bytecode[index]) + " is not a valid setting.");
		index += 1 + sizeof(size_t);
	}
	++index;

	// Actual code.
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

		// Optimised.
		case Instructions::CLS:
			buffer += "[-]";
			break;
		case Instructions::SCNL:
			buffer += "[<]";
			break;
		case Instructions::SCNR:
			buffer += "[>]";
			break;
		case Instructions::ADDC:
			buffer += std::string(bytecode[++index], '+');
			break;
		case Instructions::SUBC:
			buffer += std::string(bytecode[++index], '-');
			break;
		case Instructions::MOVLC:
			buffer += std::string(bytecode[++index], '<');
			break;
		case Instructions::MOVRC:
			buffer += std::string(bytecode[++index], '>');
			break;
		case Instructions::SAV:
		{
			// Check for multiplication loop. This will take place for MULA/MULS.
			// JE, SAV
			std::string loop;
			size_t tempIndex = index;
			if (tempIndex < 9 || (Instructions)bytecode[tempIndex - 9] != Instructions::JE)
				break;
			++tempIndex;

			// MOVL/MOVR/MOVLC/MOVRC, MULA/MULS
			size_t fromInitial = 0;
			bool left;
			if ((Instructions)bytecode[tempIndex] == Instructions::MOVL || (Instructions)bytecode[tempIndex] == Instructions::MOVLC)
				left = true;
			else if ((Instructions)bytecode[tempIndex] == Instructions::MOVR || (Instructions)bytecode[tempIndex] == Instructions::MOVRC)
				left = false;
			else break;
			while (left && ((Instructions)bytecode[tempIndex] == Instructions::MOVL || (Instructions)bytecode[tempIndex] == Instructions::MOVLC) || !left && ((Instructions)bytecode[tempIndex] == Instructions::MOVR || (Instructions)bytecode[tempIndex] == Instructions::MOVRC))
			{
				unsigned char count = 1;
				nextInstruction(bytecode, tempIndex, &count);
				fromInitial += count;
				loop += std::string(count, left ? '<' : '>');
				if ((Instructions)bytecode[tempIndex] != Instructions::MULA && (Instructions)bytecode[tempIndex] != Instructions::MULS)
					break;
				Instructions mul = nextInstruction(bytecode, tempIndex, &count);
				loop += std::string(count, mul == Instructions::MULA ? '+' : '-');
			}

			// Move back.
			loop += std::string(fromInitial, left ? '>' : '<') + "-]";

			// MOV, CLS
			if ((Instructions)bytecode[tempIndex++] != Instructions::MOV || (Instructions)bytecode[tempIndex] != Instructions::CLS)
				break;
			
			// Finish.
			buffer += loop;
			index = tempIndex;
			break;
		}
		case Instructions::MOV:
			break;
		default:
			throw DecompilerException("Decompiler: Undefined instruction " + std::to_string(bytecode[index]));
		}
	}

	return buffer;
}
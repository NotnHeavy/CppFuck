#include <vector>
#include <stack>
#include <string>
#include <BFParser.h>

std::vector<CppFuck::Opcode> CppFuck::Parse(const char* code, const unsigned long long& length)
{
	std::vector<CppFuck::Opcode> opcodes;
	std::stack<size_t> loops;
	unsigned long long line = 1, column = 1;
	const char* currentCharacter = code;
	size_t start = (size_t)currentCharacter;

	for (; (size_t)currentCharacter < start + length; ++currentCharacter)
	{
		switch (*currentCharacter)
		{
		case '+':
			opcodes.push_back({ Registers::ADD, line, column });
			break;
		case '-':
			opcodes.push_back({ Registers::SUB, line, column });
			break;
		case '>':
			opcodes.push_back({ Registers::MOVR, line, column });
			break;
		case '<':
			opcodes.push_back({ Registers::MOVL, line, column });
			break;
		case ',':
			opcodes.push_back({ Registers::IN, line, column });
			break;
		case '.':
			opcodes.push_back({ Registers::OUT, line, column });
			break;
		case '[':
			loops.push(opcodes.size());
			opcodes.push_back({ Registers::JE, line, column });
			break;
		case ']':
		{
			if (loops.empty())
				throw ParserException("Parser: No opening bracket at " + std::to_string(line) + ":" + std::to_string(column));
			size_t index = loops.top();
			loops.pop();
			opcodes[index].Offset = opcodes.size();
			opcodes.push_back({ Registers::JNE, line, column, index });
			break;
		}
		case '\n':
			++line;
			column = 0;
			goto Advance;
		default:
			goto Advance;
		}

		Advance:
		++column;
	}

	if (!loops.empty())
		throw ParserException("Parser: No closing bracket at " + std::to_string(line) + ":" + std::to_string(column));

	return opcodes;
}
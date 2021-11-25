#include <vector>
#include <stack>
#include <string>
#include <BFParser.h>

std::vector<CppFuck::Opcode> CppFuck::Parse(const char* code, const unsigned long long& length)
{
	std::vector<CppFuck::Opcode> opcodes;
	const char* currentCharacter = code;
	size_t start = (size_t)currentCharacter, loopCount = 0, line = 1, column = 1;

	for (; (size_t)currentCharacter < start + length; ++currentCharacter)
	{
		switch (*currentCharacter)
		{
		case '+':
			opcodes.push_back({ Instructions::ADD, line, column });
			break;
		case '-':
			opcodes.push_back({ Instructions::SUB, line, column });
			break;
		case '>':
			opcodes.push_back({ Instructions::MOVR, line, column });
			break;
		case '<':
			opcodes.push_back({ Instructions::MOVL, line, column });
			break;
		case ',':
			opcodes.push_back({ Instructions::IN, line, column });
			break;
		case '.':
			opcodes.push_back({ Instructions::OUT, line, column });
			break;
		case '[':
			++loopCount;
			opcodes.push_back({ Instructions::JE, line, column });
			break;
		case ']':
		{
			if (!loopCount)
				throw ParserException("Parser: No opening bracket at " + std::to_string(line) + ":" + std::to_string(column));
			--loopCount;
			opcodes.push_back({ Instructions::JNE, line, column });
			break;
		}
		case '\n':
			++line;
			column = 0;
			break;
		}
		++column;
	}

	if (loopCount)
		throw ParserException("Parser: No closing bracket at " + std::to_string(line) + ":" + std::to_string(column));

	return opcodes;
}
#include <vector>
#include <stack>
#include <iostream>
#include <BFParser.h>
#include <BFCompiler.h>

unsigned char* CppFuck::CompileToCppFuck(const std::vector<Opcode>& opcodes, size_t& index)
{
	size_t size = 512;
	index = 0;
	std::stack<size_t> jumps;
	unsigned char* buffer = new unsigned char[size] { 0 };

	for (const Opcode& opcode : opcodes)
	{
		if (index + ((opcode.Token == Instructions::JE || opcode.Token == Instructions::JNE) ? 1 + sizeof(size_t) : 1) > size)
		{
			unsigned char* newBuffer = new unsigned char[size * 2] { 0 };
			memmove(newBuffer, buffer, size);
			size *= 2;
			delete[] buffer;
			buffer = newBuffer;
		}
		buffer[index] = (unsigned char)opcode.Token;
		++index;
		if (opcode.Token == Instructions::JE)
		{
			jumps.push(index);
			index += sizeof(size_t);
		}
		else if (opcode.Token == Instructions::JNE)
		{
			// Uses little endian.
			size_t lastJE = jumps.top();
			jumps.pop();
			for (size_t i = 0; i < sizeof(size_t); i++) // JE
				buffer[lastJE + i] = ((index + sizeof(size_t)) >> i * 8) & 0xFF;
			for (size_t i = 0; i < sizeof(size_t); i++) // JNE
			{
				buffer[index] = ((lastJE + sizeof(size_t)) >> i * 8) & 0xFF;
				++index;
			}
		}
	}

	if (index != size)
	{
		unsigned char* newBuffer = new unsigned char[index];
		memmove(newBuffer, buffer, index);
		delete[] buffer;
		buffer = newBuffer;
	}
	return buffer;
}
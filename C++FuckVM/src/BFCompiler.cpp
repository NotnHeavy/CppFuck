#include <vector>
#include <stack>
#include <iostream>
#include <unordered_map>
#include <BFParser.h>
#include <BFCompiler.h>

struct jump
{
	CppFuck::Instructions Instruction;
	size_t Index;
};

unsigned char* CppFuck::CompileToCppFuck(const std::vector<Opcode> opcodes, size_t& index, std::vector<Setting> settings)
{
	size_t size = 512;
	index = 0;
	std::stack<size_t> jumps;
	jump jumpForInstructions = {Instructions::NUL, 0};
	unsigned char* buffer = new unsigned char[size] {};

	// Compile configuration.
	for (const Setting& setting : settings)
	{
		if (index + 2 + sizeof(size_t) > size)
		{
			unsigned char* newBuffer = new unsigned char[size * 2]{ 0 };
			memmove(newBuffer, buffer, size);
			size *= 2;
			delete[] buffer;
			buffer = newBuffer;
		}
		buffer[index++] = (unsigned char)setting.Type;
		for (size_t i = 0; i < sizeof(size_t); i++) // JE
			buffer[index++] = static_cast<unsigned char>(setting.Value >> i * 8);
	}
	buffer[index++] = 0;

	// Compile code.
	for (const Opcode& opcode : opcodes)
	{
		if (index + 1 + sizeof(size_t) > size)
		{
			unsigned char* newBuffer = new unsigned char[size * 2] { 0 };
			memmove(newBuffer, buffer, size);
			size *= 2;
			delete[] buffer;
			buffer = newBuffer;
		}
		buffer[index++] = (unsigned char)opcode.Token;
		if (jumpForInstructions.Instruction == opcode.Token)
		{
			for (size_t i = 0; i < sizeof(size_t); i++)
				buffer[jumpForInstructions.Index + i] = static_cast<unsigned char>(index >> i * 8);
			jumpForInstructions.Instruction = Instructions::NUL;
		}
	    if (opcode.Token == Instructions::JE)
		{
			if (opcode.Count == 0)
				jumps.push(index);
			else
				jumpForInstructions = { (Instructions)opcode.Count, index };
			index += sizeof(size_t);
		}
		else if (opcode.Token == Instructions::JNE)
		{
			// Uses little endian.
			size_t lastJE = jumps.top();
			jumps.pop();
			for (size_t i = 0; i < sizeof(size_t); i++) // JE
				buffer[lastJE + i] = static_cast<unsigned char>((index + sizeof(size_t)) >> i * 8);
			for (size_t i = 0; i < sizeof(size_t); i++) // JNE
			{
				buffer[index] = static_cast<unsigned char>((lastJE + sizeof(size_t)) >> i * 8);
				++index;
			}
		}
		else if (opcode.Count > 1 || opcode.Token == Instructions::MULA || opcode.Token == Instructions::MULS)
			buffer[index++] = opcode.Count;
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
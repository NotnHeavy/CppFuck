#include <vector>
#include <stack>
#include <iostream>
#include <unordered_map>
#include <BFParser.h>
#include <BFCompiler.h>
#include <BFlib.h>

struct jump
{
	CppFuck::Instructions Instruction;
	size_t Index;
};

CppFuck::CompiledInfo CppFuck::CompileToCppFuck(const std::vector<Opcode> opcodes, std::vector<Setting> settings)
{
	size_t size = 512, debugSize = 512, index = 0, debugIndex = 0;
	std::stack<size_t> jumps;
	jump jumpForInstructions = {Instructions::NUL, 0};
	unsigned char* buffer = new unsigned char[size] {}, *debug = new unsigned char[debugSize] {};
	bool errorOnOutOfBounds = true;

	// Compile configuration.
	for (const Setting& setting : settings)
	{
		cpprealloc<unsigned char>(&buffer, size, size * 2, index + 2 + sizeof(size_t) > size);
		if (setting.Type == Configuration::OutOfBoundsBehaviour)
			errorOnOutOfBounds = setting.Value == 0 ? true : false;
		buffer[index++] = (unsigned char)setting.Type;
		for (size_t i = 0; i < sizeof(size_t); i++) // JE
			buffer[index++] = static_cast<unsigned char>(setting.Value >> i * 8);
	}
	buffer[index++] = 0;

	// Compile code.
	for (const Opcode& opcode : opcodes)
	{
		cpprealloc<unsigned char>(&buffer, size, size * 2, index + 1 + sizeof(size_t) > size);
		// This is weird but there's just a lot of conditions.
		if
		(
			(
				opcode.Token == Instructions::MOVL ||
				opcode.Token == Instructions::MOVR ||
				((unsigned char)opcode.Token > 9 && (unsigned char)opcode.Token  < 14) || // SCNL, SCNR, MULA, MULS
				opcode.Token == Instructions::MOVLC ||
				opcode.Token == Instructions::MOVRC ||
				opcode.Token == Instructions::MOV
			)
			&& errorOnOutOfBounds
		)
		{
			cpprealloc<unsigned char>(&debug, debugSize, debugSize * 2, debugIndex + sizeof(size_t) * 3 > debugSize);
			for (size_t i = 0; i < sizeof(size_t); i++) // Bytecode index.
				debug[debugIndex++] = static_cast<unsigned char>(index >> i * 8);
			for (size_t i = 0; i < sizeof(size_t); i++) // Opcode line.
				debug[debugIndex++] = static_cast<unsigned char>(opcode.Line >> i * 8);
			for (size_t i = 0; i < sizeof(size_t); i++) // Opcode column.
				debug[debugIndex++] = static_cast<unsigned char>(opcode.Column >> i * 8);
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
				buffer[index++] = static_cast<unsigned char>((lastJE + sizeof(size_t)) >> i * 8);
		}
		else if (opcode.Count > 1 || opcode.Token == Instructions::MULA || opcode.Token == Instructions::MULS || opcode.Token == Instructions::SCNL || opcode.Token == Instructions::SCNR)
			buffer[index++] = opcode.Count;
	}

	cpprealloc<unsigned char>(&buffer, size, index, index != size);
	cpprealloc<unsigned char>(&debug, debugSize, debugIndex, debugIndex != debugSize);
	return CompiledInfo( buffer, debug, index, debugIndex );
 }
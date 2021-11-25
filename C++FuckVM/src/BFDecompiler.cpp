#include <vector>
#include <BFParser.h>
#include <BFDecompiler.h>

unsigned char* CppFuck::DecompileToBF(const unsigned char* const bytecode, const size_t& size, size_t& index)
{
	size_t bufferSize = 512;
	size_t fileIndex = 0;
	index = 0;
	unsigned char* buffer = new unsigned char[bufferSize] { 0 };

	for (; fileIndex < size; ++fileIndex)
	{
		if (index + 1 > bufferSize)
		{
			unsigned char* newBuffer = new unsigned char[bufferSize * 2] { 0 };
			memmove(newBuffer, buffer, bufferSize);
			bufferSize *= 2;
			delete[] buffer;
			buffer = newBuffer;
		}
		switch ((Instructions)bytecode[fileIndex])
		{
		case Instructions::ADD:
			buffer[index] = '+';
			break;
		case Instructions::SUB:
			buffer[index] = '-';
			break;
		case Instructions::MOVL:
			buffer[index] = '<';
			break;
		case Instructions::MOVR:
			buffer[index] = '>';
			break;
		case Instructions::IN:
			buffer[index] = ',';
			break;
		case Instructions::OUT:
			buffer[index] = '.';
			break;
		case Instructions::JE:
			buffer[index] = '[';
			fileIndex += sizeof(size_t);
			break;
		case Instructions::JNE:
			buffer[index] = ']';
			fileIndex += sizeof(size_t);
			break;
		}
		++index;
	}

	if (index != bufferSize)
	{
		unsigned char* newBuffer = new unsigned char[index];
		memmove(newBuffer, buffer, index);
		delete[] buffer;
		buffer = newBuffer;
	}
	return buffer;
}
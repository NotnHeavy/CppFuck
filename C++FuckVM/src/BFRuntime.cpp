#include <vector>
#include <iostream>
#include <string>
#include <BFParser.h>
#include <BFCompiler.h>
#include <BFRuntime.h>
#include <BFlib.h>

// there seems to be a slight overhead now. previous mandelbrot time on release: 5990ms. now: 8000ms

enum class eofHandler : unsigned char
{
	NoChange = 0,
	Minus = 1,
	Zero = 2
};

enum class outOfBoundsHandler : unsigned char
{
	Error = 0,
	Expand = 1,
	Wrap = 2
};

static __forceinline void grabValue(unsigned char* buffer, const size_t& length, size_t& index, size_t& value, bool usememcpy = false)
{
	if (usememcpy)
		memcpy(&value, buffer + index, sizeof(size_t));
	else
	{
		for (size_t i = 0; i < sizeof(size_t) && index < length; ++i)
			value |= static_cast<size_t>(buffer[index++]) << i * 8;
	}
}

static __forceinline bool grabValue(CppFuck::CompiledInfo& info, size_t& index)
{
	size_t value = 0;
	for (size_t i = 0; i < sizeof(size_t) && index < info.BytecodeLength; ++i)
		value |= static_cast<size_t>(info.Bytecode[index++]) << i * 8;
	return value;
}

static void throwError(std::string error, size_t& bytecodeIndex, size_t& size, size_t& begin, unsigned char* buffer, CppFuck::CompiledInfo& info)
{
	buffer = (unsigned char*)begin;
	delete[] buffer;
	size_t line = 0, column = 0;
	if (info.DebugCode != nullptr)
	{
		size_t index = 0;
		while (index + 0 < info.DebugCodeLength)
		{
			size_t bytecodeIndexFound = 0, lineFound = 0, columnFound = 0;
			grabValue(info.DebugCode, info.DebugCodeLength, index, bytecodeIndexFound);
			grabValue(info.DebugCode, info.DebugCodeLength, index, lineFound);
			grabValue(info.DebugCode, info.DebugCodeLength, index, columnFound);
			if (bytecodeIndex == bytecodeIndexFound)
			{
				line = lineFound, column = columnFound;
				break;
			}
		}

	}
	throw CppFuck::RuntimeException("Runtime: " + error + "\n\nBuffer size: " + std::to_string(size) + "\n" + (line != 0 ? "Line " + std::to_string(line) + " : " + std::to_string(column) : "Bytecode index: " + std::to_string(bytecodeIndex)), size, line != 0 ? line : bytecodeIndex, column);
}

void CppFuck::InitiateVM(CompiledInfo& info, std::istream& in, std::ostream& out)
{
	// Analyse configuration.
	size_t index = 0, size = 30000;
	bool optimiseMemoryCopying = true;
	eofHandler eof = eofHandler::Zero;
	outOfBoundsHandler outOfBounds = outOfBoundsHandler::Error;
	while (index < info.BytecodeLength)
	{
		switch ((Configuration)info.Bytecode[index])
		{
		case Configuration::EoF:
			++index;
			goto startRuntime;
		case Configuration::SetBufferSize:
			++index;
			size = 0;
			grabValue(info.Bytecode, info.BytecodeLength, index, size);
			break;
		case Configuration::OutOfBoundsBehaviour:
		{
			++index;
			size_t value = 0;
			grabValue(info.Bytecode, info.BytecodeLength, index, value);
			if (value > 2)
				throw CppFuck::RuntimeException(std::to_string(value) + " is not a valid option for setting OutOfBoundsBehaviour.", 0, 0, 0);
			outOfBounds = (outOfBoundsHandler)value;
			break;
		}
		case Configuration::OptimiseMemoryCopying:
			++index;
			optimiseMemoryCopying = grabValue(info, index);
			break;
		case Configuration::EoFHandling:
		{
			++index;
			size_t value = 0;
			grabValue(info.Bytecode, info.BytecodeLength, index, value);
			if (value > 2)
				throw CppFuck::RuntimeException(std::to_string(value) + " is not a valid option for setting EoFHandling.", 0, 0, 0);
			eof = (eofHandler)value;
			break;
		}
		default:
			throw CppFuck::RuntimeException(std::to_string(info.Bytecode[index]) + " is not a valid setting.", 0, 0, 0);
		}
	}

	// Set up buffer.
	startRuntime:
	unsigned char* buffer = new unsigned char[size] { 0 };
	size_t begin = (size_t)buffer, addressRegister = 0;

	// Run code.
	while (index < info.BytecodeLength)
	{
		switch ((Instructions)info.Bytecode[index])
		{
		case Instructions::ADD:
			++*buffer;
			break;
		case Instructions::SUB:
			--*buffer;
			break;
		case Instructions::MOVL:
			if ((size_t)buffer - 1 < begin)
			{
				if (outOfBounds == outOfBoundsHandler::Expand)
				{
					cpprealloc<unsigned char>(&buffer, size, size * 2, true, &begin, size);
					buffer = (unsigned char*)(begin + size / 2 - 1);
					break;
				}
				else if (outOfBounds == outOfBoundsHandler::Wrap)
				{
					buffer = (unsigned char*)(begin + size - 1);
					break;
				}
				else
					throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - 1)), index, size, begin, buffer, info);
			}
			--buffer;
			break;
		case Instructions::MOVR: // need to fix.
			if ((size_t)buffer + 1 > begin + size - 1)
			{
				if (outOfBounds == outOfBoundsHandler::Expand)
				{
					cpprealloc<unsigned char>(&buffer, size, size * 2, true, &begin);
					buffer = (unsigned char*)(begin + size / 2);
					break;
				}
				else if (outOfBounds == outOfBoundsHandler::Wrap)
				{
					buffer = (unsigned char*)begin;
					break;
				}
				else
					throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: " + std::to_string((size_t)buffer - begin + 1), index, size, begin, buffer, info);
			}
			++buffer;
			break;

		// Optimised instructions.
		case Instructions::ADDC:
			*buffer += info.Bytecode[++index];
			break;
		case Instructions::SUBC:
			*buffer -= info.Bytecode[++index];
			break;
		case Instructions::MOVLC:
		{
			unsigned char value = info.Bytecode[++index];
			if ((size_t)buffer - value < begin)
			{
				size_t offset = (size_t)buffer - begin + size;
				if (outOfBounds == outOfBoundsHandler::Expand)
				{
					expandl:
					cpprealloc<unsigned char>(&buffer, size, size * 2, true, &begin, size);
					buffer = (unsigned char*)(begin + offset);
					if ((size_t)buffer - value < begin)
						goto expandl;
				}
				else if (outOfBounds == outOfBoundsHandler::Wrap)
				{
					buffer = (unsigned char*)(begin + (offset + size - value) % size);
					break;
				}
				else
					throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - value)), --index, size, begin, buffer, info);
			}
			buffer -= value;
			break;
		}
		case Instructions::MOVRC:
		{
			unsigned char value = info.Bytecode[++index];
			if ((size_t)buffer + value > begin + size - 1)
			{
				size_t offset = (size_t)buffer - begin;
				if (outOfBounds == outOfBoundsHandler::Expand)
				{
					expandr:
					cpprealloc<unsigned char>(&buffer, size, size * 2, true, &begin);
					buffer = (unsigned char*)(begin + offset);
					if ((size_t)buffer + value > begin + size - 1)
						goto expandr;
				}
				else if (outOfBounds == outOfBoundsHandler::Wrap)
				{
					buffer = (unsigned char*)(begin + (offset + value) % size);
					break;
				}
				else
					throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: " + std::to_string((size_t)buffer + value - begin), --index, size, begin, buffer, info);
			}
			buffer += value;
			break;
		}
		case Instructions::IN:
		{
			in >> std::noskipws >> *buffer;
			if (in.eof())
				*buffer = eof != eofHandler::NoChange ? (unsigned char)eof - 2 : *buffer;
			else if (*buffer == 0x0d && in.peek() == 0x0a)
			{
				// CRLF to LF
				*buffer = 0x0a;
				in.seekg(1, std::ios::cur);
			}
			break;
		}
		case Instructions::OUT:
#if defined(_WIN32) || defined(_WIN64)
			// LF to CRLF
			if (*buffer == 0x0a)
				out << "\r\n";
			else
#endif
			out << *buffer;
			break;
		case Instructions::JE:
			if (!*buffer)
			{
				size_t tempIndex = index + 1;
				index = 0;
				grabValue(info.Bytecode, info.BytecodeLength, tempIndex, index, optimiseMemoryCopying);
				continue;
			}
			else 
				index += sizeof(size_t);
			break;
		case Instructions::JNE:
			if (*buffer)
			{
				size_t tempIndex = index + 1;
				index = 0;
				grabValue(info.Bytecode, info.BytecodeLength, tempIndex, index, optimiseMemoryCopying);
				continue;
			}
			else 
				index += sizeof(size_t);
			break;
		case Instructions::CLS:
			*buffer = 0;
			break;
		case Instructions::SCNL:
		{
			unsigned char count = info.Bytecode[++index];
			while (*buffer)
			{
				if ((size_t)buffer - count < begin)
				{
					size_t offset = (size_t)buffer - begin + size;
					if (outOfBounds == outOfBoundsHandler::Expand)
					{
						expandscnl:
						cpprealloc<unsigned char>(&buffer, size, size * 2, true, &begin, size);
						buffer = (unsigned char*)(begin + offset);
						if ((size_t)buffer - count < begin)
							goto expandscnl;
					}
					else if (outOfBounds == outOfBoundsHandler::Wrap)
						buffer = (unsigned char*)(begin + (offset + size - count) % size);
					else
						throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - count)), --index, size, begin, buffer, info);
				}
				buffer -= count;
			}
			break;
		}
		case Instructions::SCNR:
		{
			unsigned char count = info.Bytecode[++index];
			while (*buffer)
			{
				if ((size_t)buffer + count > begin + size - 1)
				{
					size_t offset = (size_t)buffer - begin;
					if (outOfBounds == outOfBoundsHandler::Expand)
					{
						expandscnr:
						cpprealloc<unsigned char>(&buffer, size, size * 2, true, &begin);
						buffer = (unsigned char*)(begin + offset);
						if ((size_t)buffer + count > begin + size)
							goto expandscnr;
					}
					else if (outOfBounds == outOfBoundsHandler::Wrap)
						buffer = (unsigned char*)(begin + (offset + count) % size);
					else
						throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: " + std::to_string((size_t)buffer - begin + count) + (std::string)" at bytecode index ", --index, size, begin, buffer, info);
				}
				buffer += count;
			}
			break;
		}
		case Instructions::MULA:
		{
			++index;
			size_t value = addressRegister;
			while (value > size - 1)
			{
				if (outOfBounds == outOfBoundsHandler::Expand)
				{
					size_t offset = (size_t)buffer - begin;
					cpprealloc<unsigned char>(&buffer, size, size * 2, true, &begin, size);
					buffer = (unsigned char*)(begin + offset);
				}
				else if (outOfBounds == outOfBoundsHandler::Wrap)
					value %= size;
				else
					throwError("Dereference of illegal address stored in address register, where address register index is set to " + std::to_string(addressRegister), --index, size, begin, buffer, info);
			}
			*buffer += *(unsigned char*)(begin + value) * info.Bytecode[index];
			break;
		}
		case Instructions::MULS:
		{
			++index;
			size_t value = addressRegister;
			while (value > size - 1)
			{
				if (outOfBounds == outOfBoundsHandler::Expand)
				{
					size_t offset = (size_t)buffer - begin;
					cpprealloc<unsigned char>(&buffer, size, size * 2, true, &begin, size);
					buffer = (unsigned char*)(begin + offset);
				}
				else if (outOfBounds == outOfBoundsHandler::Wrap)
					value %= size;
				else
					throwError("Dereference of illegal address stored in address register. Address index is set to " + std::to_string(addressRegister), --index, size, begin, buffer, info);
			}
			*buffer -= *(unsigned char*)(begin + value) * info.Bytecode[index];
			break;
		}
		case Instructions::SAV:
			addressRegister = (size_t)buffer - begin;
			break;
		case Instructions::MOV:
		{
			size_t value = addressRegister;
			while (value > size - 1)
			{
				if (outOfBounds == outOfBoundsHandler::Expand)
				{
					size_t offset = (size_t)buffer - begin;
					cpprealloc<unsigned char>(&buffer, size, size * 2, true, &begin, size);
					buffer = (unsigned char*)(begin + offset);
				}
				else if (outOfBounds == outOfBoundsHandler::Wrap)
					value %= size;
				else
					throwError("Undefined behaviour prevented from illegal address stored in address register. Address index is set to " + std::to_string(addressRegister), index, size, begin, buffer, info);
			}
			buffer = (unsigned char*)(begin + value);
			break;
		}
		default:
			std::string binary = "0b";
			for (int i = sizeof(info.Bytecode[index]) * 8 - 1; i > -1; --i)
				binary += std::to_string((int)(info.Bytecode[index] >> i) & 1);
			throwError("Undefined instruction " + binary, index, size, begin, buffer, info);
		}
		++index;
	}

	buffer = (unsigned char*)begin;
	delete[] buffer;
	std::cout << std::endl;
}
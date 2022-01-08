#include <vector>
#include <iostream>
#include <string>
#include <BFParser.h>
#include <BFCompiler.h>
#include <BFRuntime.h>

enum class EoFHandler : unsigned char
{
	NoChange = 0,
	Minus = 1,
	Zero = 2
};

enum class OutOfBoundsHandler : unsigned char
{
	Error = 0,
	Expand = 1,
	Wrap = 2
};

static inline void throwError(std::string error, size_t& bytecodeIndex, size_t& size, size_t& begin, unsigned char* buffer)
{
	buffer = (unsigned char*)begin;
	delete[] buffer;
	throw CppFuck::RuntimeException("Runtime: " + error + "\n\nBuffer size: " + std::to_string(size) + "\nBytecode index: " + std::to_string(bytecodeIndex)); // Pretty simple.
}

static inline void grabValue(const unsigned char* const code, size_t& index, const size_t& length, size_t& value, bool usememcpy = false)
{
	if (usememcpy)
		memcpy(&value, code + index + 1, sizeof(size_t));
	else
	{
		for (size_t i = 0; i < sizeof(size_t) && index < length; ++i)
			value |= static_cast<size_t>(code[++index]) << i * 8;
	}
}

static inline bool grabValue(const unsigned char* const code, size_t& index, const size_t& length, bool usememcpy = false)
{
	size_t value = 0;
	for (size_t i = 0; i < sizeof(size_t) && index < length; ++i)
		value |= static_cast<size_t>(code[++index]) << i * 8;
	return value;
}

void CppFuck::InitiateVM(const unsigned char* const code, const size_t length, std::istream& in, std::ostream& out) // may consider allowing user to specify ostream/istream as second parameter.
{
	// Analyse configuration.
	size_t index = 0, size = 30000;
	bool /*dynamicallyExpanding = false, wrapAround = false, */wrapAroundCell = true, optimiseMemoryCopying = false;
	EoFHandler eof = EoFHandler::Zero;
	OutOfBoundsHandler outOfBounds = OutOfBoundsHandler::Error;
	while (index < length)
	{
		switch ((Configuration)code[index])
		{
		case Configuration::EoF:
			++index;
			goto startRuntime;
		case Configuration::SetBufferSize:
			grabValue(code, index, length, size);
			break;
		case Configuration::OutOfBoundsBehaviour:
		{
			size_t value = 0;
			grabValue(code, index, length, value);
			if (value > 2)
				throw CppFuck::RuntimeException(std::to_string(value) + " is not a valid option for setting OutOfBoundsBehaviour.");
			outOfBounds = (OutOfBoundsHandler)value;
			break;
		}
		case Configuration::OptimiseMemoryCopying:
			optimiseMemoryCopying = grabValue(code, index, length);
			break;
		case Configuration::EoFHandling:
		{
			size_t value = 0;
			grabValue(code, index, length, value);
			if (value > 2)
				throw CppFuck::RuntimeException(std::to_string(value) + " is not a valid option for setting EoFHandling.");
			eof = (EoFHandler)value;
			break;
		}
		default:
			throw CppFuck::RuntimeException(std::to_string(code[index]) + " is not a valid setting.");
		}
		++index;
	}

	// Set up buffer.
	startRuntime:
	unsigned char* buffer = new unsigned char[size] { 0 };
	size_t begin = (size_t)buffer, addressRegister = 0;

	// Run code.
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
			{
				if (outOfBounds == OutOfBoundsHandler::Expand)
				{
					buffer = (unsigned char*)begin;
					unsigned char* newBuffer = new unsigned char[size * 2]{ };
					memmove(&newBuffer[size], buffer, size);
					size *= 2;
					delete[] buffer;
					buffer = newBuffer;
					begin = (size_t)buffer;
					buffer = (unsigned char*)(begin + size / 2 - 1);
					break;
				}
				else if (outOfBounds == OutOfBoundsHandler::Wrap)
				{
					buffer = (unsigned char*)(begin + size - 1);
					break;
				}
				else
					throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - 1)), index, size, begin, buffer);
			}
			--buffer;
			break;
		case Instructions::MOVR: // need to fix.
			if ((size_t)buffer + 1 > begin + size - 1)
			{
				if (outOfBounds == OutOfBoundsHandler::Expand)
				{
					buffer = (unsigned char*)begin;
					unsigned char* newBuffer = new unsigned char[size * 2]{ };
					memmove(newBuffer, buffer, size);
					size *= 2;
					delete[] buffer;
					buffer = newBuffer;
					begin = (size_t)buffer;
					buffer = (unsigned char*)(begin + size / 2);
					break;
				}
				else if (outOfBounds == OutOfBoundsHandler::Wrap)
				{
					buffer = (unsigned char*)begin;
					break;
				}
				else
					throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: " + std::to_string((size_t)buffer - begin), index, size, begin, buffer);
			}
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
			{
				size_t offset = (size_t)buffer - begin + size;
				if (outOfBounds == OutOfBoundsHandler::Expand)
				{
					expandl:
					buffer = (unsigned char*)begin;
					unsigned char* newBuffer = new unsigned char[size * 2]{ };
					memmove(&newBuffer[size], buffer, size);
					size *= 2;
					delete[] buffer;
					buffer = newBuffer;
					begin = (size_t)buffer;
					buffer = (unsigned char*)(begin + offset);
					if ((size_t)buffer - value < begin)
						goto expandl;
				}
				else if (outOfBounds == OutOfBoundsHandler::Wrap)
				{
					buffer = (unsigned char*)(begin + (offset + size - value) % size);
					break;
				}
				else
					throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - value)), index, size, begin, buffer);
			}
			buffer -= value;
			break;
		}
		case Instructions::MOVRC:
		{
			unsigned char value = code[++index];
			if ((size_t)buffer + value > begin + size - 1)
			{
				size_t offset = (size_t)buffer - begin;
				if (outOfBounds == OutOfBoundsHandler::Expand)
				{
					expandr:
					buffer = (unsigned char*)begin;
					unsigned char* newBuffer = new unsigned char[size * 2]{ };
					memmove(newBuffer, buffer, size);
					size *= 2;
					delete[] buffer;
					buffer = newBuffer;
					begin = (size_t)buffer;
					buffer = (unsigned char*)(begin + offset);
					if ((size_t)buffer + value > begin + size - 1)
						goto expandr;
				}
				else if (outOfBounds == OutOfBoundsHandler::Wrap)
				{
					buffer = (unsigned char*)(begin + (offset + value) % size);
					break;
				}
				else
					throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: " + std::to_string((size_t)buffer + value - 1 - begin), index, size, begin, buffer);
			}
			buffer += value;
			break;
		}
		case Instructions::IN:
		{
			in >> std::noskipws >> *buffer;
			if (in.eof())
				*buffer = eof != EoFHandler::NoChange ? (unsigned char)eof - 2 : *buffer;
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
				grabValue(code, index, length, index, optimiseMemoryCopying);
				continue;
			}
			else 
				index += sizeof(size_t);
			break;
		case Instructions::JNE:
			if (*buffer)
			{
			    grabValue(code, index, length, index, optimiseMemoryCopying);
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
					throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: -" + std::to_string(begin - ((size_t)buffer - 1)), index, size, begin, buffer);
				--buffer;
			}
			break;
		case Instructions::SCNR:
			while (*buffer)
			{
				if ((size_t)buffer + 1 > begin + size)
					throwError("Undefined behaviour prevented from out-of-bounds pointer. Illegal buffer index: " + std::to_string((size_t)buffer - begin) + (std::string)" at bytecode index ", index, size, begin, buffer);
				++buffer;
			}
			break;
		case Instructions::MULA:
		{
			++index;
			size_t value = addressRegister;
			while (value > size - 1)
			{
				if (outOfBounds == OutOfBoundsHandler::Expand)
				{
					size_t offset = (size_t)buffer - begin;
					buffer = (unsigned char*)begin;
					unsigned char* newBuffer = new unsigned char[size * 2]{ };
					memmove(&newBuffer[size], buffer, size);
					size *= 2;
					delete[] buffer;
					buffer = newBuffer;
					begin = (size_t)buffer;
					buffer = (unsigned char*)(begin + offset);
				}
				else if (outOfBounds == OutOfBoundsHandler::Wrap)
					value %= size;
				else
					throwError("Dereference of illegal address stored in address register, where address register index is set to " + std::to_string(addressRegister), index, size, begin, buffer);
			}
			if (!wrapAroundCell && *buffer + code[++index] <= *buffer)
				throwError("Integral overflow at cell " + std::to_string((size_t)buffer - begin) + ": value cannot exceed value 255", index, size, begin, buffer);
			*buffer += *(unsigned char*)(begin + value) * code[index];
			break;
		}
		case Instructions::MULS:
		{
			++index;
			size_t value = addressRegister;
			while (value > size - 1)
			{
				if (outOfBounds == OutOfBoundsHandler::Expand)
				{
					size_t offset = (size_t)buffer - begin;
					buffer = (unsigned char*)begin;
					unsigned char* newBuffer = new unsigned char[size * 2]{ };
					memmove(&newBuffer[size], buffer, size);
					size *= 2;
					delete[] buffer;
					buffer = newBuffer;
					begin = (size_t)buffer;
					buffer = (unsigned char*)(begin + offset);
				}
				else if (outOfBounds == OutOfBoundsHandler::Wrap)
					value %= size;
				else
					throwError("Dereference of illegal address stored in address register. Address index is set to " + std::to_string(addressRegister), index, size, begin, buffer);
			}
			*buffer -= *(unsigned char*)(begin + value) * code[index];
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
				if (outOfBounds == OutOfBoundsHandler::Expand)
				{
					size_t offset = (size_t)buffer - begin;
					buffer = (unsigned char*)begin;
					unsigned char* newBuffer = new unsigned char[size * 2]{ };
					memmove(&newBuffer[size], buffer, size);
					size *= 2;
					delete[] buffer;
					buffer = newBuffer;
					begin = (size_t)buffer;
					buffer = (unsigned char*)(begin + offset);
				}
				else if (outOfBounds == OutOfBoundsHandler::Wrap)
					value %= size;
				else
					throwError("Undefined behaviour prevented from illegal address stored in address register. Address index is set to " + std::to_string(addressRegister), index, size, begin, buffer);
			}
			buffer = (unsigned char*)(begin + value);
			break;
		}
		default:
			std::string binary = "0b";
			for (int i = sizeof(code[index]) * 8 - 1; i > -1; --i)
				binary += std::to_string((int)(code[index] >> i) & 1);
			throwError("Undefined instruction " + binary, index, size, begin, buffer);
		}
		++index;
	}

	buffer = (unsigned char*)begin;
	delete[] buffer;
	std::cout << std::endl;
}
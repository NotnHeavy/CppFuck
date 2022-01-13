// todo: allow substraction for mul loops, allow multiple </> for scan loops

#include <vector>
#include <BFParser.h>
#include <BFOptimiser.h>

static size_t advance(CppFuck::Instructions instruction, std::vector<CppFuck::Opcode>& opcodes, size_t& index, bool* limit, bool ignoreLimit = false)
{
	size_t count = 0;
	while (opcodes[index].Token == instruction)
	{
		++index; ++count;
		if (count > 255 && !ignoreLimit)
		{
			if (limit != nullptr)
				*limit = true;
			break;
		}
	}
	return count;
}

static bool checkForClear(std::vector<CppFuck::Opcode>& opcodes, std::vector<CppFuck::Opcode>& opcode, size_t& index)
{
	size_t line = opcodes[index].Line, column = opcodes[index].Column, tempIndex = index;
	++tempIndex;
	if (opcodes[tempIndex].Token != CppFuck::Instructions::ADD && opcodes[tempIndex].Token != CppFuck::Instructions::SUB)
		return false;
	advance(opcodes[tempIndex].Token, opcodes, tempIndex, nullptr);
	if (opcodes[tempIndex].Token != CppFuck::Instructions::JNE)
		return false;
	opcode.push_back({ CppFuck::Instructions::CLS });
	index = ++tempIndex;
	return true;
}

static bool checkForScan(std::vector<CppFuck::Opcode>& opcodes, std::vector<CppFuck::Opcode>& opcode, size_t& index)
{
	size_t line = opcodes[index].Line, column = opcodes[index].Column, tempIndex = index;
	++tempIndex;
	if ((opcodes[tempIndex].Token != CppFuck::Instructions::MOVL && opcodes[tempIndex].Token != CppFuck::Instructions::MOVR) || opcodes[tempIndex + 1].Token != CppFuck::Instructions::JNE || opcodes[tempIndex].Count != 1)
		return false;
	index = tempIndex + 2;
	opcode.push_back(opcodes[tempIndex].Token == CppFuck::Instructions::MOVL ? CppFuck::Opcode { CppFuck::Instructions::SCNL, line, column } : CppFuck::Opcode{ CppFuck::Instructions::SCNR, line, column });
	return true;
}

static bool checkForMultiplication(std::vector<CppFuck::Opcode>& opcodes, std::vector<CppFuck::Opcode>& opcode, size_t& index)
{
	// For comments for loop progress: () means optional, {} means either one but one is needed.

	// Work our way through the beginning of the loop.
	// [(-) 
	// add opcode here for jumping past CLS opcode.
	opcode.push_back({ CppFuck::Instructions::JE, opcodes[index].Line, opcodes[index].Column, (unsigned char)CppFuck::Instructions::CLS });
	opcode.push_back({ CppFuck::Instructions::SAV, opcodes[index].Line, opcodes[index].Column });
	size_t tempIndex = index;
	++tempIndex;
	CppFuck::Instructions initialClear = CppFuck::Instructions::NUL; // just force user to use sub only for now.
	if (opcodes[tempIndex].Token == CppFuck::Instructions::SUB/* || opcodes[tempIndex].Token == CppFuck::Instructions::ADD*/)
	{
		initialClear = opcodes[tempIndex].Token == CppFuck::Instructions::ADD ? CppFuck::Instructions::SUB : CppFuck::Instructions::SUB;
		++tempIndex;
	}
	if (opcodes[tempIndex].Token != CppFuck::Instructions::MOVR && opcodes[tempIndex].Token != CppFuck::Instructions::MOVL)
		return false;
	size_t fromInitialIndex = 0;
	CppFuck::Instructions initialShift = opcodes[tempIndex].Token;
	CppFuck::Instructions backShift = initialShift == CppFuck::Instructions::MOVL ? CppFuck::Instructions::MOVR : CppFuck::Instructions::MOVL;

	// Check for multiplications.
	// {<>}{+-} until {<>} not found
	while (opcodes[tempIndex].Token == initialShift)
	{
		bool limit = false;
		size_t line = opcodes[tempIndex].Line, column = opcodes[tempIndex].Column;

		unsigned char count = static_cast<unsigned char>(advance(initialShift, opcodes, tempIndex, &limit));
		if (limit)
			return false;
		opcode.push_back({ count > 1 ? (CppFuck::Instructions)((unsigned char)opcodes[tempIndex - 1].Token + 13) : opcodes[tempIndex - 1].Token, line, column, count });
		fromInitialIndex += count;
		if (opcodes[tempIndex].Token != CppFuck::Instructions::ADD && opcodes[tempIndex].Token != CppFuck::Instructions::SUB)
			return false;

		count = static_cast<unsigned char>(advance(opcodes[tempIndex].Token, opcodes, tempIndex, &limit));
		if (limit)
			return false;
		opcode.push_back({ opcodes[tempIndex - 1].Token == CppFuck::Instructions::ADD ? CppFuck::Instructions::MULA : CppFuck::Instructions::MULS, opcodes[tempIndex].Line, opcodes[tempIndex].Column, count });
	}

	// Clear up.
	// {<>}{+-}]
	if (opcodes[tempIndex].Token != backShift || advance(backShift, opcodes, tempIndex, nullptr, true) != fromInitialIndex)
		return false;
	if (initialClear == CppFuck::Instructions::NUL && opcodes[tempIndex++].Token != CppFuck::Instructions::SUB || opcodes[tempIndex++].Token != CppFuck::Instructions::JNE)
		return false;
	opcode.push_back({ CppFuck::Instructions::MOV });
	opcode.push_back({ CppFuck::Instructions::CLS });
	index = tempIndex;
	return true;
}

std::vector<CppFuck::Opcode> CppFuck::Optimise(std::vector<Opcode> opcodes)
{
	std::vector<Opcode> optimised;
	size_t index = 0;
	Instructions lastInstruction = Instructions::NUL;

	while (index < opcodes.size())
	{
		if ((unsigned char)opcodes[index].Token > 0 && (unsigned char)opcodes[index].Token < 5)
		{
			size_t line = opcodes[index].Line, column = opcodes[index].Column;
			unsigned char count = 1;
			++index;
			while (index < opcodes.size() && opcodes[index].Token == opcodes[index - 1].Token && count < 255)
			{
				++count;
				++index;
			}
			Opcode opcode = { count > 1 ? (CppFuck::Instructions)((unsigned char)opcodes[index - 1].Token + 13) : opcodes[index - 1].Token, line, column, count };
			optimised.push_back(opcode);
			lastInstruction = opcode.Token;
			continue;
		}
		else if (opcodes[index].Token == Instructions::JE)
		{
			std::vector<Opcode> opcode;
			if (checkForClear(opcodes, opcode, index) || checkForScan(opcodes, opcode, index) || checkForMultiplication(opcodes, opcode, index))
			{
				if (lastInstruction == opcode[0].Token && opcode[0].Token == Instructions::CLS)
					continue;
				lastInstruction = opcode.back().Token;
				optimised.reserve(opcode.size());
				optimised.insert(optimised.end(), opcode.begin(), opcode.end());
				continue;
			}
			else
			{
				lastInstruction = opcodes[index].Token;
				optimised.push_back(opcodes[index]);
			}
		}
		else
		{
			lastInstruction = opcodes[index].Token;
			optimised.push_back(opcodes[index]);
		}

		++index;
	}
	return optimised;
}
#include <iostream>
#include <string>
#include <chrono>
#include <version.h>
#include <BFParser.h>
#include <BFRuntime.h>

#include <stdio.h>

static void run(const char* const code, const unsigned long long& length)
{
	try
	{
		// Create new timepoint.
		std::chrono::steady_clock::time_point current = std::chrono::high_resolution_clock::now();

		std::vector<CppFuck::Opcode> opcodes = CppFuck::Parse(code, length);
		CppFuck::InitiateVM(opcodes);

		// Prints elapsed time in milliseconds.
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - current).count() << "ms" << std::endl;
	}
	catch (const CppFuck::BaseCppFuckException& exception)
	{
		std::cout << exception.what() << std::endl;
	}
}

int main(int argc, char* argv[])
{
	if (argc > 1) // Load file.
	{
		FILE* file;
		if (fopen_s(&file, argv[1], "r") != 0)
		{
			std::cout << "The file \"" << argv[1] << "\" does not exist on your system." << std::endl;
			return 1;
		}
		fseek(file, 0, SEEK_END);
		const unsigned long long size = ftell(file);
		char* contents = new char[size];
		fseek(file, 0, SEEK_SET);
		fread(contents, size, 1, file);
		fclose(file);
		run(contents, size);
		delete[] contents;
	}
	else // Interactive mode.
	{
		std::cout << "C++Fuck " << FVERSION_STRINGIZED << " main (compiled on " << __DATE__ << ", " << __TIME__ << " with MSVC, for win64)" << std::endl;
		std::string input;
		while (true)
		{
			std::cout << ">>> ";
			std::getline(std::cin, input);
			run(input.c_str(), input.size());
		}
	}
}
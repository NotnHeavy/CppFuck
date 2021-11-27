#include <iostream>
#include <string>
#include <chrono>
#include <map>
#include <version.h>
#include <BFParser.h>
#include <BFCompiler.h>
#include <BFRuntime.h>
#include <BFDecompiler.h>

static int run(const char* const code, const unsigned long long& length)
{
	try
	{
		// Create new timepoint and length variable.
		const std::chrono::steady_clock::time_point current = std::chrono::high_resolution_clock::now();
		size_t compiledLength;

		const std::vector<CppFuck::Opcode> opcodes = CppFuck::Parse(code, length);
		unsigned char* output = CppFuck::CompileToCppFuck(opcodes, compiledLength);
		CppFuck::InitiateVM(output, compiledLength);

		// Prints elapsed time in milliseconds.
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - current).count() << "ms" << std::endl;
		delete[] output;
	}
	catch (const CppFuck::BaseCppFuckException& exception)
	{
		std::cout << exception.what() << std::endl;
		return 1;
	}
	return 0;
}

static const std::map<const std::string, const std::string> commands
{
	{ "-o", "Compile C++Fuck bytecode for a Brainfuck script and create an output file for it." },
	{ "-run", "Initiate runtime for C++Fuck bytecode. This is unnecessary if no other arguments are being used." }
};

int main(int argc, char* argv[])
{
	if (argc > 1) // Load file.
	{
		if ((std::string)argv[1] == "-?")
		{
			std::cout << "Usage: C++Fuck file [options]\n\nAll options:\n-?: Displays help information." << std::endl;
			for (const std::pair<const std::string, const std::string>& command : commands)
			{
				std::cout << command.first << ": " << command.second << std::endl;
			}
		}
		else
		{
			// Load file.
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

			// Parse options.
			if (argc < 3)
				return run(contents, size);
			else
			{
				bool running = false;
				std::vector<std::string> used;
				for (size_t i = 2; i < argc; ++i)
				{
					if (commands.count(argv[i]))
					{
						if (std::find(used.begin(), used.end(), argv[i]) != used.end())
						{
							std::cout << "The argument \"" << argv[i] << "\" has been used already." << std::endl;
							return 1;
						}
						std::string argument = argv[i];
						used.push_back(argument);
						if (argument == "-o")
						{
							++i;
							std::string location = (std::string)argv[i] + ".bfc";
							try
							{
								FILE* file;
								if (fopen_s(&file, location.c_str(), "w") != 0)
								{
									std::cout << "The file \"" << argv[i] << "\" could not be created. Check your output directory." << std::endl;
									return 1;
								}
								size_t compiledLength;
								const std::vector<CppFuck::Opcode> opcodes = CppFuck::Parse(contents, size);
								unsigned char* output = CppFuck::CompileToCppFuck(opcodes, compiledLength);
								fwrite(output, compiledLength, 1, file);
								fclose(file);
							}
							catch (const CppFuck::BaseCppFuckException& exception)
							{
								std::cout << exception.what() << std::endl;
								return 1;
							}
						}
						else if (argument == "-run")
							running = true;
					}
					else
					{
						std::cout << "The argument \"" << argv[i] << "\" does not exist. Use -? to display the help menu." << std::endl;
						return 1;
					}
				}
				if (running)
					return run(contents, size);
			}
			delete[] contents;
		}
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
// todo: (maybe) revert vector item allocation to stack.

#include <iostream>
#include <string>
#include <chrono>
#include <map>
#include <sstream>
#include <version.h>
#include <BFParser.h>
#include <BFOptimiser.h>
#include <BFCompiler.h>
#include <BFRuntime.h>
#include <BFDecompiler.h>

// 0.13:
// implement basic debugging, i.e. create file containing line/column info. file should use file extension .bfcd.

// 0.14:
// configuration parameters for compilation

// .bfc = brainfuck code
// .bfcd = brainfuck code debug info

static inline bool checkIfBytecode(std::string path)
{
	size_t position = path.find_last_of('.');
	if (position != std::string::npos && path.substr(position + 1) == "bfc")
		return true;
	else
		return false;
}

static int run(const unsigned char* const code, const unsigned long long& length, bool bytecode = false, bool showExecTime = true)
{
	unsigned char* output = nullptr;
	try
	{
		// Create new timepoint and length variable.
		const std::chrono::steady_clock::time_point current = std::chrono::high_resolution_clock::now();
		size_t compiledLength;

		if (bytecode)
			CppFuck::InitiateVM(code, length, std::cin, std::cout);
		else
		{
			
			std::vector<CppFuck::Opcode> opcodes = CppFuck::Parse(code, length);
			opcodes = CppFuck::Optimise(opcodes);
			output = CppFuck::CompileToCppFuck(opcodes, compiledLength, { { CppFuck::Configuration::OptimiseMemoryCopying, true } });
			CppFuck::InitiateVM(output, compiledLength, std::cin, std::cout);
		}

		// Prints elapsed time in milliseconds.
		if (showExecTime)
			std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - current).count() << "ms" << std::endl;
		delete[] output;
	}
	catch (const CppFuck::BaseCppFuckException& exception)
	{
		delete[] output;
		std::cerr << exception.what() << std::endl;
		return 1;
	}
	return 0;
}

static const std::map<const std::string, const std::string> commands
{
	{ "-o", "Compile C++Fuck bytecode for a Brainfuck script and create an output file for it." },
	{ "-run", "Initiate runtime for C++Fuck bytecode. This is unnecessary if no other arguments are being used." },
	{ "-dec", "Decompile C++Fuck bytecode to a specific output."},
	{ "-postexec", "Output time taken to execute code." }
};

int main(int argc, char* argv[])
{
	if (argc > 1) // Load file.
	{
		std::string firstArg = argv[1];
		if (firstArg == "-?")
		{
			std::cout << "Usage: C++Fuck [file] [options]\n\nAll options:\n-?: Displays help information." << std::endl;
			for (const std::pair<const std::string, const std::string>& command : commands)
				std::cout << command.first << ": " << command.second << std::endl;
		}
		else
		{
			// Load file.
			bool bytecode = checkIfBytecode(firstArg);
			FILE* file;
			if (fopen_s(&file, argv[1], "rb") != 0)
			{
				std::cerr << "The file \"" << argv[1] << "\" does not exist on your system." << std::endl;
				return 1;
			}
			_fseeki64(file, 0, SEEK_END);
			const unsigned long long size = _ftelli64(file);
			unsigned char* contents = new unsigned char[size];
			_fseeki64(file, 0, SEEK_SET);
			fread(contents, size, 1, file);
			fclose(file);

			// Parse options.
			if (argc < 3)
				return run(contents, size, bytecode, false);
			else
			{
				const std::chrono::steady_clock::time_point current = std::chrono::high_resolution_clock::now();
				bool running = false;
				bool showExecTime = false;
				std::vector<std::string> used;
				for (size_t i = 2; i < argc; ++i)
				{
					if (commands.count(argv[i]))
					{
						if (std::find(used.begin(), used.end(), argv[i]) != used.end())
						{
							std::cerr << "The argument \"" << argv[i] << "\" has been used already." << std::endl;
							return 1;
						}
						std::string argument = argv[i];
						used.push_back(argument);
						if (argument == "-o")
						{
							if (bytecode)
							{
								std::cerr << "The file \"" << argv[i] << "\" is already compiled bytecode." << std::endl;
								return 1;
							}
							++i;
							std::string location = (std::string)argv[i] + ".bfc";
							FILE* file = nullptr;
							unsigned char* output = nullptr;
							try
							{
								if (fopen_s(&file, location.c_str(), "wb") != 0)
								{
									std::cerr << "The file \"" << argv[i] << "\" could not be created. Verify your output directory." << std::endl;
									return 1;
								}
								size_t compiledLength;
								std::vector<CppFuck::Opcode> opcodes = CppFuck::Parse(contents, size);
								opcodes = CppFuck::Optimise(opcodes);
								output = CppFuck::CompileToCppFuck(opcodes, compiledLength);
								fwrite(output, compiledLength, 1, file);
								fclose(file);
							}
							catch (const CppFuck::BaseCppFuckException& exception)
							{
								std::cout << exception.what() << std::endl;
								delete[] contents;
								delete[] output;
								fclose(file);
								remove(location.c_str());
								return 1;
							}
						}
						else if (argument == "-run")
							running = true;
						else if (argument == "-postexec")
							showExecTime = true;
						else if (argument == "-dec")
						{
							++i;
							std::string location = (std::string)argv[i] + ".bf";
							FILE* file = nullptr;
							try
							{
								if (fopen_s(&file, location.c_str(), "wb") != 0)
								{
									std::cerr << "The file \"" << argv[i] << "\" could not be created. Verify your output directory." << std::endl;
									return 1;
								}
								std::cout << std::endl;
								std::string output = CppFuck::DecompileToBF(contents, size);
								fwrite(output.c_str(), output.length(), 1, file);
								fclose(file);
							}
							catch (const CppFuck::BaseCppFuckException& exception)
							{
								std::cout << exception.what() << std::endl;
								delete[] contents;
								fclose(file);
								remove(location.c_str());
								return 1;
							}
						}
					}
					else
					{
						std::cerr << "The argument \"" << argv[i] << "\" does not exist. Use -? to display the help menu." << std::endl;
						return 1;
					}
				}
				if (running)
					return run(contents, size, bytecode, false);
				if (showExecTime)
					std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - current).count() << "ms" << std::endl;
			}
			delete[] contents;
		}
	}
	else // Interactive mode.
	{
		std::cout << "C++Fuck " << FVERSION_STRINGIZED << " " << GIT_BRANCH << " (compiled on " << __DATE__ << ", " << __TIME__ << " with MSVC, for win64)" << std::endl;
		std::string input;
		while (true)
		{
			std::cout << ">>> ";
			std::getline(std::cin, input);
			if (input.substr(0, 3) == "run")
			{
				std::string location = input.substr(3);
				location.erase(location.begin(), std::find_if(location.begin(), location.end(), [](char space) { return !isspace(space); }));
				bool bytecode = checkIfBytecode(location);
				FILE* file;
				if (fopen_s(&file, location.c_str(), "rb") != 0)
				{
					std::cerr << "The file \"" << location << "\" does not exist on your system." << std::endl;
					continue;
				}
				_fseeki64(file, 0, SEEK_END);
				const unsigned long long size = _ftelli64(file);
				unsigned char* contents = new unsigned char[size];
				_fseeki64(file, 0, SEEK_SET);
				fread(contents, size, 1, file);
				fclose(file);
				run(contents, size, bytecode);
				delete[] contents;
			}
			else
				run(reinterpret_cast<unsigned char*>(const_cast<char*>(input.c_str())), input.size());
		}
	}
}
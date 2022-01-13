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

// 0.14:
// configuration parameters for compilation.
// clean up code, eliminate bugs.

static inline bool checkIfBytecode(std::string path)
{
	size_t position = path.find_last_of('.');
	if (position != std::string::npos && path.substr(position + 1) == "bfc")
		return true;
	else
		return false;
}

static int run(const unsigned char* const code, const unsigned long long& length, bool showExecTime = true, CppFuck::CompiledInfo* info = nullptr)
{
	unsigned char* output = nullptr;
	try
	{
		// Create new timepoint and length variable.
		const std::chrono::steady_clock::time_point current = std::chrono::high_resolution_clock::now();


		if (info)
			CppFuck::InitiateVM(*info, std::cin, std::cout);
		else
		{
			std::vector<CppFuck::Opcode> opcodes = CppFuck::Parse(code, length);
			opcodes = CppFuck::Optimise(opcodes);
			CppFuck::CompiledInfo info = CppFuck::CompileToCppFuck(opcodes, { { CppFuck::Configuration::OptimiseMemoryCopying, true } });
			CppFuck::InitiateVM(info, std::cin, std::cout);
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

static CppFuck::CompiledInfo getBytecode(std::string location, unsigned char* bytecode = nullptr, size_t bytecodeSize = 0)
{
	// File variables.
	FILE* file;
	unsigned char* debugCode = nullptr;
	size_t debugCodeSize = 0;
	location.erase(std::find_if(location.rbegin(), location.rend(), [](char space) { return !isspace(space); }).base(), location.end());

	// Get bytecode.
	if (!bytecode)
	{
		if (fopen_s(&file, location.c_str(), "rb") != 0)
		{
			std::cerr << "The file \"" << location << "\" does not exist on your system." << std::endl;
			return CppFuck::CompiledInfo();
		}
		_fseeki64(file, 0, SEEK_END);
		bytecodeSize = _ftelli64(file);
		bytecode = new unsigned char[bytecodeSize];
		_fseeki64(file, 0, SEEK_SET);
		fread(bytecode, bytecodeSize, 1, file);
		fclose(file);
		file = nullptr;
	}

	// Check for debug code.
	if (fopen_s(&file, (location + "d").c_str(), "rb") != 0)
		return CppFuck::CompiledInfo(bytecode, nullptr, bytecodeSize);
	_fseeki64(file, 0, SEEK_END);
	debugCodeSize = _ftelli64(file);
	debugCode = new unsigned char[debugCodeSize];
	_fseeki64(file, 0, SEEK_SET);
	fread(debugCode, debugCodeSize, 1, file);
	fclose(file);
	return CppFuck::CompiledInfo(bytecode, debugCode, bytecodeSize, debugCodeSize);
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
			int error = 0;
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
			{
				if (bytecode)
				{
					CppFuck::CompiledInfo info = getBytecode(argv[1], contents, size);
					return run(contents, size, false, &info);
				}
				else
					error = run(contents, size, false);
			}
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
							std::string location = argv[i];
							FILE* file = nullptr;
							try
							{
								if (fopen_s(&file, (location + ".bfc").c_str(), "wb") != 0)
								{
									std::cerr << "The file \"" << (location + ".bfc") << "\" could not be created. Verify your output directory." << std::endl;
									return 1;
								}
								std::vector<CppFuck::Opcode> opcodes = CppFuck::Parse(contents, size);
								opcodes = CppFuck::Optimise(opcodes);
								CppFuck::CompiledInfo info = CppFuck::CompileToCppFuck(opcodes);
								fwrite(info.Bytecode, info.BytecodeLength, 1, file);
								fclose(file);
								file = nullptr;
								if (fopen_s(&file, (location + ".bfcd").c_str(), "wb") != 0)
								{
									std::cerr << "The file \"" << (location + ".bfcd") << "\" could not be created. Verify your output directory." << std::endl;
									return 1;
								}
								fwrite(info.DebugCode, info.DebugCodeLength, 1, file);
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
									std::cerr << "The file \"" << location << "\" could not be created. Verify your output directory." << std::endl;
									return 1;
								}
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
					error = run(contents, size, bytecode);
				if (showExecTime)
					std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - current).count() << "ms" << std::endl;
			}
			delete[] contents;
			return error;
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
				if (bytecode)
				{
					CppFuck::CompiledInfo info = getBytecode(location, contents, size);
					run(nullptr, size, true, &info);
				}
				else
				{
					run(contents, size, true, nullptr);
					delete[] contents;
				}
			}
			else
				run(reinterpret_cast<unsigned char*>(const_cast<char*>(input.c_str())), input.size(), true);
		}
	}
}
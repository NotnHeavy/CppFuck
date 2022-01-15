// todo: (maybe) revert vector item allocation to stack.

#define CPPFUCK_VERSION "C++Fuck " << FVERSION_STRINGIZED << " " << GIT_BRANCH << " (compiled on " << __DATE__ << ", " << __TIME__ << " with MSVC, for win64). Licensed with MIT."

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

static inline bool checkIfBytecode(std::string path)
{
	size_t position = path.find_last_of('.');
	if (position != std::string::npos && path.substr(position + 1) == "bfc")
		return true;
	else
		return false;
}

static int run(const unsigned char* const code, const unsigned long long& length, bool showExecTime = true, CppFuck::CompiledInfo* info = nullptr, std::vector<CppFuck::Setting> settings = { }, bool optimise = true)
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
			if (optimise)
				opcodes = CppFuck::Optimise(opcodes);
			CppFuck::CompiledInfo info = CppFuck::CompileToCppFuck(opcodes, settings);
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
		std::cerr << "\n\n" << exception.what() << std::endl;
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

struct command
{
	const std::string Name, Parameters, Description;
	command(const std::string name, const std::string parameters, const std::string description)
		: Name(name), Parameters(parameters), Description(description)
	{
	}
};

static const std::vector<command> commands
{
	{ "-o", "[file]", "Compile C++Fuck bytecode for a Brainfuck script and create an output file for it." },
	{ "--run", "", "Initiate runtime for C++Fuck bytecode. This is unnecessary if no other arguments are being used." },
	{ "--dec", "[file]", "Decompile C++Fuck bytecode to a specific output."},
	{ "--postexec", "", "Output time taken to execute code." },
	{ "--version", "", "Display version information." }
};

static const std::vector<command> compileCommands
{
	{ "--optimise", "[true/false]", "Enable/disable optimisation." },
	{ "--setbuffersize", "[size]", "Set the buffer size. 30000 by default." },
	{ "--outofboundsbehaviour", "[0/1/2]", "Toggle between error (0), expand the buffer automatically (1) or wrap around the buffer (2). Errors by default (0)." },
	{ "--optimisememorycopying", "[true/false]", "Toggle between robustness (bit shifts) and speed (memcpy) for copying integral values from bytecode to memory. Uses bit shifts by default." },
	{ "--eofhandling", "[0/1/2]", "Toggle between no change (0), set to -1 (1) or set to 0 (2). Sets to 0 by default (2)." }
};

static size_t evaluate(const std::string& value)
{
	if (value == "true")
		return 1;
	else if (value == "false")
		return 0;
	else
	{
		std::istringstream stream(value);
		size_t value;
		stream >> value;
		if (stream.fail())
			return std::numeric_limits<size_t>::max();
		else
			return value;
	}
}

int main(int argc, char* argv[])
{
	if (argc > 1) // Load file.
	{
		std::string firstArg = argv[1];
		if (firstArg == "-?")
		{
			std::cout << "Usage: C++Fuck [file] [options]\n\nAll options:\n-?: Displays help information." << std::endl;
			for (const command& command : commands)
				std::cout << command.Name << (command.Parameters != "" ? " " + command.Parameters : "") << ": " << command.Description << std::endl;

			std::cout << "\n" << "Parameters for code configuration:" << std::endl;
			for (const command& command : compileCommands)
				std::cout << command.Name << (command.Parameters != "" ? " " + command.Parameters : "") << ": " << command.Description << std::endl;
			std::cout << "\n" << "True/false evaluate to 1/0." << "\n" << "If the only parameter parsed is the file, it will be compiled and executed on the fly. Otherwise, you'll have to use the \"-run\" parameter as well." << std::endl;
		}
		else
		{
			// Initial CL parsing.
			size_t i = 1;
			if ((std::string)argv[i] == "-version")
			{
				std::cout << CPPFUCK_VERSION << std::endl;
				++i;
				if (argc < 3)
					return 0;
			}

			// Load file.
			int error = 0;
			bool bytecode = checkIfBytecode(firstArg);
			FILE* file;
			if (fopen_s(&file, argv[i], "rb") != 0)
			{
				std::cerr << "The file \"" << argv[i] << "\" does not exist on your system." << std::endl;
				return 1;
			}
			_fseeki64(file, 0, SEEK_END);
			const unsigned long long size = _ftelli64(file);
			unsigned char* contents = new unsigned char[size];
			_fseeki64(file, 0, SEEK_SET);
			fread(contents, size, 1, file);
			fclose(file);
			++i;

			// Parse options.
			if (argc < i + 1)
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
				bool running = false, showExecTime = false, optimise = true, compile = false, decompile = false;
				std::vector<std::string> used;
				std::vector<CppFuck::Setting> configuration;
				std::string location;
				for (; i < argc; ++i)
				{
					std::string argument = argv[i];
					//if (std::find_if(commands.begin(), commands.end(), [&argument](const command& command) { return command.Name == argument; }) != commands.end())
					if (std::find_if(commands.begin(), commands.end(), [&argument](const command& cmd) { return cmd.Name == argument; }) != commands.end())
					{
						if (std::find(used.begin(), used.end(), argv[i]) != used.end())
						{
							std::cerr << "The argument \"" << argv[i] << "\" has been used already." << std::endl;
							return 1;
						}
						used.push_back(argument);
						if (argument == "-o")
						{
							compile = true;
							location = argv[++i];
						}
						else if (argument == "--run")
							running = true;
						else if (argument == "--postexec")
							showExecTime = true;
						else if (argument == "--dec")
						{
							decompile = true;
							location = (std::string)argv[++i] + ".bf";
						}
						else if (argument == "--version")
							std::cout << CPPFUCK_VERSION << std::endl;
					}
					else
					{
						const std::vector<command>::const_iterator iterator = std::find_if(compileCommands.begin(), compileCommands.end(), [&argument](const command& command) { return command.Name == argument; });
						if (iterator == compileCommands.end())
						{
							std::cerr << "The argument \"" << argv[i] << "\" does not exist. Use -? to display the help menu." << std::endl;
							return 1;
						}
						size_t value = evaluate(argv[++i]);
					    if (argument == "--optimise")
							optimise = value;
						else
							configuration.push_back({ (CppFuck::Configuration)(size_t)(iterator - compileCommands.begin()), value });
					}
				}

				// Execute anything that should be executed last.
				if (running)
				{
					if (bytecode)
					{
						CppFuck::CompiledInfo info = getBytecode(argv[1], contents, size);
						error = run(contents, size, showExecTime, &info, configuration, optimise);
					}
					else
						error = run(contents, size, showExecTime, nullptr, configuration, optimise);
					showExecTime = false;
				}
				if (compile)
				{
					// File variables.
					if (bytecode)
					{
						std::cerr << "The file \"" << location << "\" is already compiled bytecode." << std::endl;
						return 1;
					}
					FILE* file = nullptr;

					// Compile code.
					try
					{
						if (fopen_s(&file, (location + ".bfc").c_str(), "wb") != 0)
						{
							std::cerr << "The file \"" << (location + ".bfc") << "\" could not be created. Verify your output directory." << std::endl;
							return 1;
						}
						std::vector<CppFuck::Opcode> opcodes = CppFuck::Parse(contents, size);
						if (optimise)
							opcodes = CppFuck::Optimise(opcodes);
						CppFuck::CompiledInfo info = CppFuck::CompileToCppFuck(opcodes, configuration);
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
				if (decompile)
				{
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
				if (showExecTime)
					std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - current).count() << "ms" << std::endl;
			}
			delete[] contents;
			return error;
		}
	}
	else // Interactive mode.
	{
		std::cout << CPPFUCK_VERSION << std::endl;
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
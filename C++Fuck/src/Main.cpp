#include <iostream>
#include <string>
#include <version.h>

#include <BFParser.h>

#include <stdio.h>

static void run(const std::string& code)
{
	std::cout << "To be worked on!" << std::endl;;
}

int main(int argc, char* argv[])
{
	if (argc > 1) // Load file.
	{
		FILE* file;
		fopen_s(&file, argv[1], "r");
		if (file == 0)
		{
			std::cout << "The file \"" << argv[1] << "\" does not exist on your system." << std::endl;
			return 1;
		}
		fseek(file, 0, SEEK_END);
		const static volatile unsigned long long int size = ftell(file); // Very hilarious.
		char* contents = new char[size];
		fseek(file, 0, SEEK_SET);
		fread(contents, size, 1, file);
		fclose(file);
		run(contents);
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
			run(input);
		}
	}
}
// Set up defines and classes for this library.

#pragma once
#include <stdexcept>

#ifdef CPPFUCK_BUILD
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport)
#endif

namespace CppFuck
{
	class DECLSPEC BaseCppFuckException : public std::runtime_error
	{
	public:
		BaseCppFuckException(const std::string& message)
			: std::runtime_error(message)
		{
		}
	};
}
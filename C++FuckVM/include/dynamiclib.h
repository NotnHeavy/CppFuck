// Set up defines and classes for this library.

#pragma once
#include <stdexcept>

#ifdef CPPFUCK_BUILD
#define CPPFUCK_API __declspec(dllexport)
#else
#define CPPFUCK_API __declspec(dllimport)
#endif

namespace CppFuck
{
	class CPPFUCK_API BaseCppFuckException : public std::runtime_error
	{
	public:
		BaseCppFuckException(const std::string& message)
			: std::runtime_error(message)
		{
		}
	};
}
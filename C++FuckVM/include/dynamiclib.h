// Pretty much any macros used.

#pragma once

#ifdef CPPFUCK_BUILD
#define CPPFUCK_API __declspec(dllexport)
#else
#define CPPFUCK_API __declspec(dllimport)
#endif
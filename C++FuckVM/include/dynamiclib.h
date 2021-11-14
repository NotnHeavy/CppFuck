// Set up defines for this library.

#pragma once

#ifdef CPPFUCK_BUILD
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport)
#endif
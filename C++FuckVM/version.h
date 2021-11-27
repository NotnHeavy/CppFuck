#pragma once
#include <FileVersion.h>
#define return_to_string(s) #s // Stringize function.
#define to_string(s) return_to_string(s)

#define DESCRIPTION "A feature-rich Brainfuck virtual machine."
#define FVERSION V_MAJOR, V_MINOR, V_BUILD, V_REVISION // Same as the product version.
#define FVERSION_STRINGIZED to_string(V_MAJOR) "." to_string(V_MINOR) "." to_string(V_BUILD) "." to_string(V_REVISION) // Same as the stringized product version.
#define PRODUCTNAME "C++FuckVM"
#define ORIGINALNAME PRODUCTNAME ".dll" // Same as internal name.
#define COPYRIGHT "Copyright (C) 2021 NotnHeavy"

#ifdef _DEBUG
#define IS_DEBUG VS_FF_DEBUG
#else
#define IS_DEBUG 0
#endif

#define OS VOS_NT_WINDOWS32
#define FTYPE VFT_DLL // For DLLs, do VFT_DLL.

# C++Fuck
C++Fuck (otherwise known as CppFuck due to limitations) is an optimised Brainfuck virtual machine that can be easily integrated into any application.

C++Fuck is split into two projects: C++Fuck and C++FuckVM.

## C++Fuck
This is just a small wrapper for C++FuckVM that allows for quick code execution, on the fly. You can either execute code via command-line arguments, or you can use the basic REPL interface. 

The REPL interface simply allows you to run code on the fly (you can load in files with the run keyword, as so: <code>run YOURFILE.bf</code>.

There are a couple command-line arguments that you can use, for either interacting with C++FuckVM differently, or by changing how the code is compiled. You can learn more about these commands by using the <code>-?</code> parameter.

## C++FuckVM
This is where all of the main code is at. The general pipeline for execution is parsing, optimising (optional), compiling and executing. A decompiler is also present, should you need to decompile any C++Fuck bytecode. Although debugging information is present, it (currently) only maps the bytecode index to its corresponding line/column numbers.

For integration, everything is in the CppFuck namespace. Include the necessary header files (BFCompiler.h, BFDecompiler.h, BFOptimiser.h, BFParser.h, BFRuntime.h) in order to call the necessary functions for compiling and executing Brainfuck code. There are not many definitions, therefore it should be easy to navigate around.

Have fun!

### MIT (2021), created by NotnHeavy.

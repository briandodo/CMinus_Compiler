# CMinus_Compiler

This repository contains the source code for a C-Minus compiler. The compiler consists of several files that work together to perform lexical analysis, syntax analysis, semantic analysis, and code generation.

# Files

* ast.c and ast.h: These files contain the implementation and header file for the Abstract Syntax Tree (AST) data structure. The AST is built during the syntax analysis phase and is used for type checking and generating intermediate representation.
* emit.c and emit.h: These files contain the implementation and header file for the code generation module. The code generation module takes the AST produced by the syntax analysis phase and generates MIPS assembly code as intermediate representation.
* lab9.l: This file contains the lexical analysis rules for the C-Minus grammar. It defines the tokens and their corresponding regular expressions.
* lab9.y: This file contains the syntax analysis rules for the C-Minus grammar. It performs the syntax analysis, builds the AST, and performs semantic analysis by interacting with the symbol table.
* makefile: This makefile provides the necessary commands to compile the source code files and generate an executable. It ensures that all dependencies are properly handled during the build process.
* symtable.c and symtable.h: These files contain the implementation and header file for the symbol table data structure. The symbol table is used during semantic analysis to store and manage information about identifiers in the source code.

# Usage
To compile the C-Minus compiler, navigate to the project directory and run the following command:
make

This will compile all the source files and generate an executable named lab9.
To use the compiler, run the following command:
```
./lab9 <input-file>
```
Replace <input-file> with the path to the C-Minus source code file you wish to compile. The compiler will perform lexical analysis, syntax analysis, semantic analysis, and generate the corresponding MIPS assembly code as intermediate representation.

# Test Code
The test folder contains sample C-Minus source code files that can be used to test the compiler. You can provide these files as input to the compiler to observe its behavior and the generated output.

Homework 2 - Compiler

To compile:

gcc -o [executable name] compiler.c code_generator.c parser.c lexicalAnalyzer.c p_machine.c

./[executable name] [flag] [flag] [flag]

There are 3 flags available:
  -l: output the lexemes to lexemes.txt
  -a: output the assembly code to assembly.txt
  -v: output the virtual machine's execution trace to virtual_machine.txt

The program will ask you for an input file. Then, it compiles the program found
in that file and prints a success message to the screen if it is syntactically
correct. It will terminate early otherwise.

If it all compiles, it will show the generated assembly code in the terminal. If
any of the flags were used, it will create a file for each of the desired outputs.

#include <stdio.h>
#include <stdlib.h>
#include "internals.h"
#include "compiler.h"

int main(int argc, char** argv) {
  
  printf("Please enter the name of the program you wish to compile:\n");
  char program[50];
  scanf("%s", program);
  
  compile(program);

}

void compile(char* program) {

  // define output files for the parts of the compiler
  char* lex_output = "lexemes.txt";
  char* parser_output = "assembly.txt";
  char* vm_output = "virtual_machine.txt";

  // run the lexical analyzer
  run_lexical_analyzer(lex_output);

  // parse
  run_parser(parser_output);

  // show that the program is syntactically correct
  success();
  
  run_virtual_machine(vm_output);

}

void success() {
  printf("Program is syntactically correct.\n");
}


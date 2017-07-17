#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internals.h"
#include "compiler.h"

int Show_Lexemes = 0;
int Show_Assembly = 0;
int Show_Execution = 0;

// every time you call block(), create 4 slots for control variables

int main(int argc, char** argv) {
  
  // handle flags
  int num_flags = argc - 1;
  char** flags = argv;
  assign_flags(num_flags, flags);
  
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

  run_lexical_analyzer(program, lex_output);

  run_parser(parser_output);

  // program is syntactically correct
  success();
  
  run_virtual_machine(vm_output);

}

void success() {
  printf("\nProgram is syntactically correct.\n\n");
}

void assign_flags(int num_flags, char** flags) {

  if(num_flags > 3) {
    flag_error(1);
  }
  else {
    for(int i = 1; i <= num_flags; i++) {
      // check for -l
      if(strcmp(flags[i], "-l") == 0) {
        Show_Lexemes = 1;
      }
      // check for -a
      else if(strcmp(flags[i], "-a") == 0) {
        Show_Assembly = 1;
      }
      // check for -v
      else if(strcmp(flags[i], "-v") == 0) {
        Show_Execution = 1;
      }
      else {
        flag_error(2);
      }
    }
  }
}

void flag_error(int num) {
  switch(num) {
    case 1:
      printf("Wrong number of flags. Maximum of 3.\n");
    case 2:
      printf("Wrong flags. Only -l, -a, and -v are allowed.\n");
  }
}

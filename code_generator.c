#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internals.h"

// create another line of assembly code
void emit(int operation, int level, int modifier) {
  // check for too much code
  if(Code_Index > MAX_CODE_LENGTH) {
    error(26);
  }
  else {
    code[Code_Index].op_code = operation;
    code[Code_Index].level = level;
    code[Code_Index].modifier = modifier;
    Code_Index++;
  }
}

// return the lexicographical level of this symbol
int symbol_level(int index) {
  return Symbol_Table[index].level;
}

// return the address of this symbol
int symbol_address(int index) {
  return Symbol_Table[index].address;
}

// return the address of this name in the symbol table, or 0 if not found
int find(char* name) {

  // navigate backwards through the symbol table
  int temp_index = Symbol_Table_Index - 1;
  while(temp_index > 0) {

    // break if a match is found
    if(strcmp(Symbol_Table[temp_index].name, name) == 0) {
      break;
    }
    
    temp_index--;
  }

  return temp_index;

}

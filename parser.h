#ifndef _PARSER_H_
#define _PARSER_H_

// internal representation of symbols
typedef enum {
  LIT = 1, OPR = 2,
  LOD = 3, STO = 4,
  CAL = 5, INC = 6,
  JMP = 7, JPC = 8,
  SIO = 9
} operation;

// print a message that the program is syntactically correct
void success();

// enter a symbol into the symbol table
void enter(int, char*, int, int, int);

// helper method to print a symbol from the symbol table
void print_symbol(int);

// helper method to print assembly code
void print_assembly(char*);

// create sample assembly code to use for integration testing
void create_sample_assembly();

// get the next token in the file from the lexeme list
void get_next_token();

// program is a block followed by a period
void program();

// block is an optional constant declaration, variable declaration, and/or statement
void block();

// declare a constant
void constant_declaration();

// declare a variable
int variable_declaration();

// declare a procedure
void procedure_declaration();

// make a statement
void statement();

// check a conditional
void condition();

// evaluate an expression
void expression();

// check for a properly defined term
void term();

// check for a proper factor
void factor();

// check for proper naming convention
void identifier();

// display an error message indicated by the number
void error(int);

// load up the lexeme list with the output from the lexical analyzer
void load_lexeme_list();

// comparison helper
bool equal(char*, int);

// verify that a token is what it is supposed to be
void verify(char*, int);

#endif
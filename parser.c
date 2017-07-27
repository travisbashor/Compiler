// Travis Bashor
// 7-7-17
// Systems Software, Homework 3: Parser with if-then-else and procedures

// includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parser.h"
#include "internals.h"

// globals
char Token[64];
char Lexeme_List[MAX_CODE_LENGTH][64];
char* id[12];
int Symbol_Table_Index = 1;
int Lexeme_List_Index = 0;
int Code_Index = 0;
int Locals_Index = 4;
int Lex_Level = 0;
Symbol Symbol_Table[MAX_SYMBOL_TABLE_SIZE];
Instruction code[MAX_CODE_LENGTH];

void run_parser(char* output_file) {

  // break up the lexeme list into an array of strings
  load_lexeme_list();

  // parse the program
  program();
   
  if(Show_Assembly == 1) {
    print_assembly(output_file);
  }
}

// <program> ::= <block>.
void program() {
  
  // move into the block
  get_next_token();

  // parse the block of code
  block();

  // check for a period at the end
  if(!equal(Token, periodsym)) {
    // period expected
    error(14);
  }

  // after the period, halt the program
  emit(SIO, 0, 3);
}

// <block> ::= <const-declaration> <var-declaration> <proc-declaration> <statement>
void block() {
  
  // increment the lexicographical level
  Lex_Level++;
  
  // jump to main(), to be modified after procedures are declared
  int jump_index = Code_Index;
  emit(JMP, 0, 0);

  // size of the stack. 4 by default
  int size = 4;
  
  // check for constant declarations
  if(equal(Token, constsym)) {
    constant_declaration();
  }
  // check for variable declarations
  if(equal(Token, varsym)) {
    size += variable_declaration();
  }
  // check for procedure declarations
  while(equal(Token, procsym)) {
    procedure_declaration();
  }

  // make sure the code index in the first jump points to the outer function
  code[jump_index].modifier = Code_Index;

  // create 4 spaces at the start for control variables, plus locals
  emit(INC, 0, size);
  
  // parse the statements
  statement();

  // decrement the lexicographical level
  Lex_Level--;
}

// <const-declaration> ::= const <ident> := <number> {, <ident> := <number>};
void constant_declaration() {

  do {

    // move to ident
    get_next_token();
    
    // check for an identifier
    if(!equal(Token, identsym)) {
      // identifier must come after keyword const
      error(2);
    }

    // move to name
    get_next_token();

    // copy the name
    char name[MAX_IDENTIFIER_LENGTH];
    strcpy(name, Token);

    // check if this name is already in the symbol table
    int symbol_index = find(name);
    if(symbol_index != 0) {
      // constant already declared
      error(3);
    }

    // move to equals
    get_next_token();

    if(!equal(Token, eqlsym)) {
      // equals must come after the identifier
      error(3);
    }
    
    // move to number
    get_next_token();

    if(!equal(Token, numbersym)) {
      // a number must come after equals
      error(2);
    }

    // move to the actual value
    get_next_token();
    
    int value = atoi(Token);

    // keep the constant in the symbol table, separate from the assembly
    enter(1, name, value, 0, 0);

    // move to comma
    get_next_token();

  } while (equal(Token, commasym));

  if(!equal(Token, semicolonsym)) {
    // expected semicolon
    error(1);
  }

  get_next_token();
}

// declare a variable
int variable_declaration() {

  // count the number of variables declared and return it to block()
  int num_variables = 0;

  do {

    // increase variable count
    num_variables++;
    
    // move to ident
    get_next_token();

    // check for ident
    if(!equal(Token, identsym)) {
      error(2);
    }

    // move to name
    get_next_token();
    
    char name[MAX_IDENTIFIER_LENGTH];
    strcpy(name, Token);

    int symbol_index = find(name);
    if(symbol_index != 0) {
      // variable already declared
      error(3);
    }
    
    // move to a comma
    get_next_token();

    // if no errors, put this symbol in the symbol table
    enter(2, name, 0, Lex_Level, Locals_Index);
    Locals_Index++;

  } while (equal(Token, commasym));

  // check for a semicolon
  if(!equal(Token, semicolonsym)) {
    error(1);
  }

  // move to whatever is beyond the semicolon
  get_next_token();

  return num_variables;
}

// procedure-declaration ::= { "procedure" ident ";" block ";" }
void procedure_declaration() {

  // move to ident
  get_next_token();

  // make sure it is ident
  if(!equal(Token, identsym)) {
    error(2);
  }

  // move to the name
  get_next_token();

  // copy the name
  char name[MAX_IDENTIFIER_LENGTH];
  strcpy(name, Token);

  // make sure this name isn't already in the symbol table
  int symbol_index = find(name);
  if(symbol_index != 0) {
    // name already in use
    error(3);
  }

  // move to semicolon
  get_next_token();

  // make sure it is a semicolon
  if(!equal(Token, semicolonsym)) {
    error(1);
  }

  // make sure we aren't nested too deep
  if(Lex_Level >= MAX_LEXI_LEVEL) {
    error(15);
  }

  // store the procedure's kind, name, val (ignored), level, and modifier in the symbol table
  enter(3, name, 0, Lex_Level, Code_Index);
  
  // move into the block
  get_next_token();

  // reset the locals index
  Locals_Index = 4;

  // parse the block
  block();

  // check for a semicolon
  if(!equal(Token, semicolonsym)) {
    error(1);
  }

  // return from this block
  emit(OPR, 0, 0);

  // move to the rest of the code
  get_next_token();

}

// make a statement
void statement() {

  // check for assignment
  if(equal(Token, identsym)) {

    // move to name
    get_next_token();

    // find by name in Symbol_Table
    int index = find(Token);
    if(index == 0) {
      // assignment of non-declared variable
      error(4);
    }
    else if(symbol_type(index) != 2) {
      // trying to write to a non-variable type
      error(10);
    }

    // move to becomes
    get_next_token();

    if(!equal(Token, becomesym)) {
      // := missing
      error(5);
    }

    // move to the expression
    get_next_token();

    // use expression to put a value on top of the stack
    expression();

    // STO whatever expression left on top of the stack by expression() into the <id>
    emit(STO, Lex_Level - symbol_level(index), symbol_address(index));

  }
  // check for procedure call
  else if(equal(Token, callsym)) {

    // move to ident
    get_next_token();

    // check for ident
    if(!equal(Token, identsym)) {
      error(9);
    }

    // move to name
    get_next_token();

    // find by name in Symbol_Table
    int index = find(Token);
    if(index == 0) {
      // call of non-declared procedure
      error(4);
    }
    else if(symbol_type(index) != 3) {
      // trying to call a non-procedure type
      error(10);
    }

    int level_difference = Lex_Level - symbol_level(index);
    if(level_difference < 0) {
      // can't call something from a deeper level
      error(16);
    }

    // call the procedure pointed to by id
    emit(CAL, level_difference, symbol_address(index));

    // move to the semi-colon or end
    get_next_token();

  }
  // check for begin/end
  else if(equal(Token, beginsym)) {

    // move to <statement-list>
    get_next_token();

    // evaluate the statement and push its result onto the stack
    statement();

    // look for more statements, delimited by semicolons
    while(equal(Token, semicolonsym)) {

      get_next_token();

      statement();
    }

    // make sure there is an end
    if(!equal(Token, endsym)) {
      // end expected
      error(6);
    }

    // if there is an end, consume the token and proceed
    get_next_token();

  }
  // check for if/then
  else if(equal(Token, ifsym)) {
    
    // move to condition
    get_next_token();

    // parse the condition
    condition();

    // make sure if is followed by then
    if(!equal(Token, thensym)) {
      // then expected
      error(7);
    }
    
    // move to the statement
    get_next_token();

    // placeholders for modifying jump after the statement
    int then_index = Code_Index;
    emit(JPC, 0, 0);

    // evaluate the statement inside then
    statement();

    // get to the bottom of the statement and tell the code to jump here
    code[then_index].modifier = Code_Index;

    if(equal(Token, elsesym)) {
      // make a placeholder
      int else_index = Code_Index;
      emit(JMP, 0, 0);

      // parse the else statement
      get_next_token();
      statement();

      // reassign the modifier of else
      code[else_index].modifier = Code_Index;

      // make the failing if condition jump into the else body
      code[then_index].modifier++;
    }


  }
  // check for while/do
  else if(equal(Token, whilesym)) {
    int temp_1 = Code_Index;

    // move to condition
    get_next_token();
    
    // parse the condition
    condition();

    // condition will leave a 1 or 0 on top of the stack, which we'll check with jump
    int temp_2 = Code_Index;

    // emit jump code as a stand-in until we have the right address
    emit(JPC, 0, 0);

    if(!equal(Token, dosym)) {
      // while must be followed by do
      error(8);
    }
    else {
      // move to statement
      get_next_token();
    }

    // put the result of statement on the top of the stack
    statement();

    emit(JMP, 0, temp_1);

    code[temp_2].modifier = Code_Index;
  }
  // check for read
  else if(equal(Token, readsym)) {

    // move to ident
    get_next_token();

    if(!equal(Token, identsym)) {
      // identifier expected
      error(9);
    }

    // move to the name
    get_next_token();

    // find the token by its name in the symbol table
    int index = find(Token);
    if(index == 0) {
      // trying to assign a variable that was never declared
      error(4);
    }
    else if(symbol_type(index) != 2) {
      // trying to assign to something that isn't a variable
      error(10);
    }

    // move to the semi-colon or next line
    get_next_token();

    // pull input from stdin and push it onto the stack
    emit(SIO, 0, 2);

    // put that value into the offset given by the symbol pointed to by identifier
    emit(STO, Lex_Level - symbol_level(index), symbol_address(index));

  }
  // print <id> to the console
  else if(equal(Token, writesym)) {

    // move to identifier
    get_next_token();

    if(!equal(Token, identsym)) {
      // identifier expected
      error(9);
    }

    // move to name
    get_next_token();

    // find the token by its name in the symbol table
    int index = find(Token);
    if(index == 0) {
      // trying to write a variable that was never declared
      error(4);
    }
    else if(symbol_type(index) == 3) {
      // trying to write a procedure
      error(10);
    }

    // move to semicolon or next line
    get_next_token();

    if(symbol_type(index) == 1) {
      // LIT the constant onto the stack
      emit(LIT, 0, symbol_value(index));
    }
    else {
      // LOD the symbol associated with <id>
      emit(LOD, Lex_Level - symbol_level(index), symbol_address(index));
    }

    // output from the top of the stack to the screen
    emit(SIO, 0, 1);

  }

}

// check a conditional
void condition() {
  if(equal(Token, oddsym)) {

    // move to the expression
    get_next_token();

    // put the result of expression on top of the stack
    expression();

    // OPR, 0, ODD
    emit(OPR, 0, 6);
    
  }
  else {

    // evaluate the left side
    expression();
    
    // check for relational operator
    if(!equal(Token, eqlsym) && !equal(Token,neqsym)
    && !equal(Token, lessym) && !equal(Token, leqsym)
    && !equal(Token, gtrsym) && !equal(Token, geqsym)) {
      // relational operator expected
      error(11);
    }
    
    char comparator[50];
    strcpy(comparator, Token);

    // move to the next operand
    get_next_token();

    // evaluate the right side
    expression();

    // emit correct assembly
    if(equal(comparator, eqlsym)) {
      emit(OPR, 0, 8);
    }
    else if(equal(comparator, neqsym)) {
      emit(OPR, 0, 9);
    }
    else if(equal(comparator, lessym)) {
      emit(OPR, 0, 10);
    }
    else if(equal(comparator, leqsym)) {
      emit(OPR, 0, 11);
    }
    else if(equal(comparator, gtrsym)) {
      emit(OPR, 0, 12);
    }
    else if(equal(comparator, geqsym)) {
      emit(OPR, 0, 13);
    }
    
  }
}

void expression() {

  int add_operation;
  if(equal(Token, plussym) || equal(Token, minussym)) {

    if(equal(Token, plussym)) {
      add_operation = plussym;
    }
    else {
      add_operation = minussym;
    }

    // move to a term
    get_next_token();

    // put the result of term() on top of the stack
    term();

    // negation
    if(add_operation == minussym) {
      emit(OPR, 0, 1);
    }

  }
  else {
    // put the result of term() on top of the stack
    term();
  }
  
  // after term(), we're either at another +/-, or 
  while(equal(Token, plussym) || equal(Token, minussym)) {

    if(equal(Token, plussym)) {
      add_operation = plussym;
    }
    else {
      add_operation = minussym;
    }

    // move to term
    get_next_token();

    // put the result of term() on top of the stack
    term();

    if(add_operation == plussym) {
      // stack[sp] += stack[sp+1]
      emit(OPR, 0, 2);
    }
    else {
      // stack[sp] -= stack[sp+1]
      emit(OPR, 0, 3);
    }
  }
  
}

// check for a properly defined term
void term() {
  
  int multi_operation;
  // store the result of factor() on top of the stack
  factor();

  while(equal(Token, multsym) || equal(Token, slashsym)) {
    // assign multi operation
    if(equal(Token, multsym)) {
      multi_operation = multsym;
    }
    else {
      multi_operation = slashsym;
    }

    // move to another factor or the end of the line
    get_next_token();

    // store the output of factor() to the top of the stack
    factor();

    // coming out of factor, we're at the end of the line or at another factor
    if(multi_operation == multsym) {
      // multiply
      emit(OPR, 0, 4);
    }
    else {
      // divide
      emit(OPR, 0, 5);
    }

  }

}

// check for a proper factor
void factor() {

  int num;

  if(equal(Token, identsym)) {

    // move to the variable name
    get_next_token();

    int index = find(Token);
    if(index == 0) {
      // undeclared identifier
      error(4);
    }

    // if it's a variable, LOD it
    if(symbol_type(index) == 2) {
      emit(LOD, Lex_Level - symbol_level(index), symbol_address(index));
    }
    // if it's a constant, LIT it
    else if (symbol_type(index) == 1) {
      emit(LIT, 0, symbol_value(index));
    }

  }
  else if(equal(Token, numbersym)) {

    // move to name
    get_next_token();

    // push the literal num to the stack
    num = atoi(Token);
    emit(LIT, 0, num);

  }
  else if(equal(Token, lparentsym)) {

    // move to the expression
    get_next_token();

    // evaluate the expression
    expression();

    // coming out of expression, we are at ")"
    if(!equal(Token, rparentsym)) {
      // missing right parenthesis
      error(12);
    }

  }
  else {
    // expected id, number, or expression
    error(13);
  }

  // move to +-, */, or ;
    get_next_token();
}

// print the assembly code
void print_assembly(char* output_file) {
  
  // open up the file given by the lexical analyzer
  FILE *ofp;
  ofp = fopen(output_file, "w");
  if (ofp == NULL) {
    fprintf(stderr, "Can't open input file.\n");
  }
  
  // print the code
  print_code(ofp, Code_Index);
}

// helper function for checking symbols in the symbol table
void print_symbol(int index) {
  
  // print a formatted token to stdout
  printf("\nKind: ");

  switch(Symbol_Table[index].kind) {
    case 1:
      printf("const\n");
    break; 
    case 2:
      printf("var\n");
    break;
    default:
      printf("Some other kind\n");
  }

  printf("Name: %s\nValue: %d\nLevel: %d\nAddress: %d\n\n",
    Symbol_Table[index].name,
    Symbol_Table[index].val,
    Symbol_Table[index].level,
    Symbol_Table[index].address);
}

// a helper to simplify conditionals
bool equal(char* token, int numerical_mapping) {
  return strcmp(token, IRMapping[numerical_mapping]) == 0;
}

// create sample assembly code output file for testing purposes
void create_sample_assembly() {
  
  Code_Index = 0;

  // create a sample output file for the VM to use as input
  int sample_assembly[] = {
    7, 0, 19,
    7, 0, 2,
    6, 0, 5,
    3, 1, 5,
    1, 0, 1,
    2, 0, 12,
    8, 0, 18,
    3, 1, 5,
    4, 0, 4,
    3, 1, 5,
    1, 0, 1,
    2, 0, 3,
    4, 1, 5,
    5, 1, 1,
    3, 1, 4,
    3, 0, 4,
    2, 0, 4,
    4, 1, 4,
    2, 0, 0,
    6, 0, 6,
    1, 0, 1,
    4, 0, 4,
    1, 0, 5,
    4, 0, 5,
    5, 0, 1,
    3, 0, 4,
    9, 0, 1,
    9, 0, 3,
    9, 0, 3
  };
  
  // fill the code array with sample code
  for(int i = 0; i < 29; i++) {
    int op_index = 3 * i;
    int level_index = 3 * i + 1;
    int mod_index = 3 * i + 2;

    // assign op, level, and modifier
    int op_code = sample_assembly[op_index];
    int level = sample_assembly[level_index];
    int modifier = sample_assembly[mod_index];

    emit(op_code, level, modifier);
  }
  
}

// get the next token in the lexeme list
void get_next_token() {
  
  // pull the token from the list
  strcpy(Token, Lexeme_List[Lexeme_List_Index]);

  // increment the list index
  Lexeme_List_Index++;

}

void enter(int kind, char *name, int val, int level, int address) {
  // create a new symbol with the given information
  Symbol temp;
  temp.kind = kind;
  strcpy(temp.name, name);
  temp.val = val;
  temp.level = level;
  temp.address = address;

  // assign it to the symbol table
  Symbol_Table[Symbol_Table_Index] = temp;
  Symbol_Table_Index++;
}

// display an error message
void error(int num) {
  printf("\nError %d: ", num);
  switch(num) {
    case 1:
      printf("Expected semicolon.\n");
      break;
    case 2:
      printf("An identifier must follow const, var, or procedure.\n");
      break;
    case 3:
      printf("Constant, variable, or procedure already declared.\n");
      break;
    case 4:
      printf("Undeclared identifier.\n");
      break;
    case 5:
      printf("Assignment operator expected.\n");
      break;
    case 6:
      printf("Keyword 'end' expected.\n");
      break;
    case 7:
      printf("Keyword 'then' expected.\n");
      break;
    case 8:
      printf("Keyword 'while' must be followed by keyword 'do'.\n");
      break;
    case 9:
      printf("Identifier expected.\n");
      break;
    case 10:
      printf("Type misuse on token: %s.\n", Token);
      break;
    case 11:
      printf("Relational operator expected.\n");
      break;
    case 12:
      printf("Missing right parenthesis.\n");
      break;
    case 13:
      printf("Expected id, factor, or expression.\n");
      break;
    case 14:
      printf("Period expected\n");
      break;
    case 15:
      printf("Procedures nested too deep. Maximum of 3 lexicographical levels allowed.\n");
      break;
    case 16:
      printf("Attempt to call a procedure from a deeper level.\nYou went too deep, man. You went too deep...\n");
  }
  printf("Parsing terminated.\n");
  exit(0);
}

void load_lexeme_list() {
  // title and token strings, for breaking apart the lexeme list
  char* title;
  char* token;

  // overlook the title
  title = strtok(symbolicLexemeList, "\n");

  // start pulling tokens from the list
  token = strtok(NULL, " ");
  
  while(token != NULL) {

    // add this token to the lexeme list
    strcpy(Lexeme_List[Lexeme_List_Index], token);

    // move forward in the list
    Lexeme_List_Index++;

    // get the next token
    token = strtok(NULL, " ");
  }
  
  // reset the index, for use later
  Lexeme_List_Index = 0;
}

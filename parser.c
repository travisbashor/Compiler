// Travis Bashor
// 7-7-17
// Systems Software, Homework 2: Parser component

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
Symbol Symbol_Table[MAX_SYMBOL_TABLE_SIZE];
Instruction code[MAX_CODE_LENGTH];

void run_parser(char* output_file) {

  // load the lexeme list into a useable array of strings
  load_lexeme_list();

  // run the parsing program
  program();
   
  if(Show_Assembly == 1) {
    print_assembly(output_file);
  }
}

// <program> ::= <block>.
void program() {
  
  get_next_token();

  block();

  // check for a period at the end
  if(!equal(Token, periodsym)) {
    // period expected
    error(9);
  }

  // after the period, halt the program
  emit(SIO, 0, 3);
}

// <block> ::= <const-declaration> <var-declaration> <statement>
void block() {
  // leave index 0 out of the program
  emit(7, 0, 1);

  // check for constant declarations
  if(equal(Token, constsym)) {

    get_next_token();
    
    constant_declaration();
  }
  // check for variable declarations
  if(equal(Token, varsym)) {

    get_next_token();

    variable_declaration();
  }
  statement();
}

// <const-declaration> ::= const <ident> := <number> {, <ident> := <number>};
void constant_declaration() {
  do {

    // create a placeholder instruction
    Instruction temp;

    // check for an identifier
    if(!equal(Token, identsym)) {
      // identifier must come after keyword const
      error(4);
    }

    // copy the name
    get_next_token();
    char name[MAX_IDENTIFIER_LENGTH];
    strcpy(name, Token);

    // check if this name is already in the symbol table
    int symbol_index = find(name);
    if(symbol_index != 0) {
      // constant already declared
      error(27);
    }

    // check for an equals sign
    get_next_token();
    if(!equal(Token, eqlsym)) {
      // equals must come after the identifier
      error(3);
    }
    
    // check for a number
    get_next_token();
    if(!equal(Token, numbersym)) {
      // a number must come after equals
      error(2);
    }
    
    // prepare for entry into symbol table
    get_next_token();
    int value = atoi(Token);

    // keep the constant in the symbol table, apart from the assembly
    enter(1, name, value, 0, 0);

    get_next_token();

  } while (equal(Token, commasym));

  // check for a semicolon
  if(!equal(Token, semicolonsym)) {
    error(5);
  }

  get_next_token();
}

// declare a variable
void variable_declaration() {
  do {

    // create a placeholder instruction
    Instruction temp;

    if(!equal(Token, identsym)) {
      // identifier must come after keyword var
      error(4);
    }

    // copy the name
    get_next_token();
    char name[MAX_IDENTIFIER_LENGTH];
    strcpy(name, Token);

    // check if this name is already in the symbol table
    int symbol_index = find(name);
    if(symbol_index != 0) {
      // variable already declared
      error(27);
    }
    
    get_next_token();

    // if no errors, put this symbol in the symbol table
    enter(2, name, 0, 0, Locals_Index);
    Locals_Index++;

    // in assembly, allocate space for the variable
    emit(INC, 0, 1);

  } while (equal(Token, commasym));

  // check for a semicolon
  if(!equal(Token, semicolonsym)) {
    error(5);
  }

  get_next_token();
}

// make a statement
void statement() {

  // check for assignment
  if(equal(Token, identsym)) {

    get_next_token();
    
    int index = find(Token);
    if(index == 0) {
      // assignment of non-declared variable
      error(11);
    }
    else if(symbol_type(index) != 2) {
      // this ain't a variable we're talkin about here
      error(12);
    }

    get_next_token();

    if(!equal(Token, becomesym)) {
      // := missing
      error(13);
    }

    get_next_token();

    expression();

    // LOD whatever expression left on top of the stack into the <id>
    emit(STO, 0, symbol_address(index));

  }
  // check for begin/end
  else if(equal(Token, beginsym)) {

    get_next_token();

    statement();

    // look for more statements, delimited by semicolons
    while(equal(Token, semicolonsym)) {

      get_next_token();

      statement();
    }

    // make sure there is an end
    if(!equal(Token, endsym)) {
      // end expected
      error(28);
    }

    // if there is an end, consume the token and proceed
    get_next_token();

  }
  // check for if/then
  else if(equal(Token, ifsym)) {
    
    get_next_token();

    // evalutate the condition
    condition();

    // make sure if is followed by then
    if(!equal(Token, thensym)) {
      // then expected
      error(30);
    }
    
    get_next_token();

    int temp_index = Code_Index;
    emit(JPC, 0, 0);

    // evaluate the statement inside then
    statement();

    // get to the bottom of the statement and tell the code to jump here
    code[temp_index].modifier = Code_Index;
  }
  // check for while/do
  else if(equal(Token, whilesym)) {
    int temp_1 = Code_Index;

    get_next_token();
    
    condition();

    int temp_2 = Code_Index;

    emit(JPC, 0, 0);

    if(!equal(Token, dosym)) {
      // while must be followed by do
      error(18);
    }

    get_next_token();

    statement();
    emit(JMP, 0, temp_1);
    code[temp_2].modifier = Code_Index;
  }
  // check for read
  else if(equal(Token, readsym)) {
    // read in the <id> and STO it
    get_next_token();

    if(!equal(Token, identsym)) {
      // identifier expected
      error(33);
    }

    get_next_token();

    // find the token by its name in the symbol table
    int index = find(Token);
    if(index == 0) {
      // trying to assign a variable that was never declared
      error(11);
    }
    else if(symbol_type(index) != 2) {
      // trying to assign to something that isn't a variable
      error(32);
    }

    get_next_token();

    // check for a semicolon
    if(!equal(Token, semicolonsym)) {
      // expected semicolon
      error(5);
    }

    // pull input from stdin and push it onto the stack
    emit(SIO, 0, 2);

    // put that value into the offset given by the symbol pointed to by identifier
    emit(STO, 0, symbol_address(index));

  }
  // print <id> to the console
  else if(equal(Token, writesym)) {

    get_next_token();

    if(!equal(Token, identsym)) {
      // identifier expected
      error(33);
    }

    get_next_token();

    // find the token by its name in the symbol table
    int index = find(Token);
    if(index == 0) {
      // trying to write a variable that was never declared
      error(11);
    }
    else if(symbol_type(index) != 2) {
      // trying to write something that isn't a variable
      error(32);
    }

    get_next_token();

    // check for a semicolon
    if(!equal(Token, semicolonsym)) {
      // expected semicolon
      error(5);
    }    

    // LOD the symbol associated with <id>
    emit(LOD, 0, Symbol_Table[index].address);

    // SIO 0 1 to output from the top of the stack to the screen
    emit(SIO, 0, 1);

  }

}

// check a conditional
void condition() {
  if(equal(Token, oddsym)) {

    get_next_token();

    expression();
  }
  else {

    expression();
    
    // check for relational operator
    if(!equal(Token, eqlsym) && !equal(Token,neqsym)
    && !equal(Token, lessym) && !equal(Token, leqsym)
    && !equal(Token, gtrsym) && !equal(Token, geqsym)) {
      // relational operator expected
      error(20);
    }

    get_next_token();

    expression();
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

    get_next_token();

    term();

    // negation
    if(add_operation == minussym) {
      // TODO: make sure the variable is what's on top of the stack
      emit(OPR, 0, 1);
    }

  }
  else {
    term();
  }
  
  while(equal(Token, plussym) || equal(Token, minussym)) {

    if(equal(Token, plussym)) {
      add_operation = plussym;
    }
    else {
      add_operation = minussym;
    }

    get_next_token();

    term();

    if(add_operation == plussym) {
      emit(OPR, 0, 2);
    }
    else {
      printf("I think I'm supposed to be subtracting...\n");
      emit(OPR, 0, 3);
    }
  }
  
}

// identify the operator
void relative_operator() {
  // useful code here
}

// check for a properly defined term
void term() {
  printf("As I'm entering term(), my symbol is %s\n", Token);
  
  int multi_operation;
  factor();

  while(equal(Token, multsym) || equal(Token, slashsym)) {

    // assign multi operation
    if(equal(Token, multsym)) {
      multi_operation = multsym;
    }
    else {
      multi_operation = slashsym;
    }
  }

  get_next_token();

  factor();

  if(multi_operation == multsym) {
    emit(OPR, 0, 4);
  }
  else {
    printf("I think I'm supposed to be dividing...\n");
    emit(OPR, 0, 5);
  }
}

// check for a proper factor
void factor() {
  if(equal(Token, identsym)) {

    get_next_token();



  }
  else if(equal(Token, numbersym)) {

    get_next_token();
    
  }
  else if(equal(Token, lparentsym)) {

    get_next_token();

    expression();

    if(!equal(Token, rparentsym)) {
      // missing right parenthesis
      error(22);
    }

    get_next_token();

  }
  else {
    // expected id, number, or expression
    error(34);
  }
}

// make sure a number is composed of digits
void number() {
  // useful code here
}

// check for proper naming convention
void identifier() {
  // useful code here
}

// check for a digit to be in the list of digit terminals
void digit() {
  // useful code here
}

// check for a character to be in the list of terminals
void letter() {
  // useful code here
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
  printf("Error %d: ", num);
  switch(num) {
    case 1:
      printf("Used = instead of :=.\n");
      break;
    case 2:
      printf("= must be followed by a number.\n");
      break;
    case 3:
      printf("Identifier must be followed by a =.\n");
      break;
    case 4:
      printf("const, var, or procedure must be followed by an identifier.\n");
      break;
    case 5:
      printf("Semicolon or comma missing.\n");
      break;
    case 6:
      printf("Incorrect symbol after procedure declaration.\n");
      break;
    case 7:
      printf("Statement expected.\n");
      break;
    case 8:
      printf("Incorrect symbol after statement part in block.\n");
      break;
    case 9:
      printf("Period expected.\n");
      break;
    case 10:
      printf("Semicolon between statements missing.\n");
      break;
    case 11:
      printf("Undeclared identifier.\n");
      break;
    case 12:
      printf("Assignment to constant or procedure is not allowed.\n");
      break;
    case 13:
      printf("Assignment operator expected.\n");
      break;
    case 14:
      printf("call must be followed by an identifier.\n");
      break;
    case 15:
      printf("Call of a constant or variable is meaningless.\n");
      break;
    case 16:
      printf("then expected.\n");
      break;
    case 17:
      printf("Semicolon or } expected.\n");
      break;
    case 18:
      printf("do expected.\n");
      break;
    case 19:
      printf("Incorrect symbol following statement.\n");
      break;
    case 20:
      printf("Relational operator expected.\n");
      break;
    case 21:
      printf("Expression must not contain a procedure identifier.\n");
      break;
    case 22:
      printf("Right parenthesis missing.\n");
      break;
    case 23:
      printf("The preceding factor cannot begin with this symbol.\n");
      break;
    case 24:
      printf("An expression cannot begin with this symbol.\n");
      break;
    case 25:
      printf("The number is too large.\n");
      break;
    case 26:
      printf("Too much code.\n");
      break;
    case 27:
      printf("Constant or variable already declared.\n");
      break;
    case 28:
      printf("Keyword end expected\n");
      break;
    case 30:
      printf("Keyword \'then\' expected");
      break;
    case 31:
      printf(":= missing in statement\n");
      break;
    case 32:
      printf("Identifier is not a variable.\n");
      break;
    case 33:
      printf("Identifier expected.\n");
      break;
    case 34:
      printf("Expected id, factor, or expression.\n");
      break;
    default:
      printf("Unknown error");
  }
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

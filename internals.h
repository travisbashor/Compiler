#ifndef _INTERNALS_H_
#define _INTERNALS_H_

// constants
#define MAX_CODE_LENGTH 32768
#define MAX_LEXI_LEVEL 3
#define MAX_NUMBER_LENGTH 5
#define MAX_IDENTIFIER_LENGTH 11
#define MAX_SYMBOL_TABLE_SIZE 2500

// instructions for the parser and vm
typedef struct {
	int op_code;
	int level;
	int modifier;
} Instruction;

// symbol to go into the symbol table
typedef struct {
  int kind;       // const = 1, var = 2, proc = 3
  char name[12];  // name, up to 11 characters
  int val;        // number (ascii value)
  int level;      // lexicographical level
  int address;    // modifier
} Symbol;

// lists for the analyzer and parser
extern char lexemeTable[MAX_CODE_LENGTH];
extern char lexemeList[MAX_CODE_LENGTH];
extern char symbolicLexemeList[MAX_CODE_LENGTH];
extern char IRMapping[34][64];
extern int Symbol_Table_Index;
extern int Code_Index;
extern int Locals_Index;
extern Instruction code[MAX_CODE_LENGTH];
extern Symbol Symbol_Table[MAX_SYMBOL_TABLE_SIZE];

// prototypes
void run_lexical_analyzer(char*, char*);
void run_parser(char*);
void run_virtual_machine(char*);
void print_code(FILE*, int);
void emit(int, int, int);
void error(int);
int find(char*);
int symbol_level(int);
int symbol_address(int);


// internal representation of symbols
typedef enum {
  nulsym = 1, identsym = 2, numbersym = 3, plussym = 4,
  minussym = 5, multsym = 6, slashsym = 7, oddsym = 8, eqlsym = 9,
  neqsym = 10, lessym = 11, leqsym = 12, gtrsym = 13, geqsym = 14,
  lparentsym = 15, rparentsym = 16, commasym = 17, semicolonsym = 18,
  periodsym = 19, becomesym = 20, beginsym = 21, endsym = 22, ifsym = 23,
  thensym = 24, whilesym = 25, dosym = 26, callsym = 27, constsym = 28,
  varsym = 29, writesym = 31, readsym = 32
} token_type;

// values for printing, given at call time
extern int Show_Lexemes;
extern int Show_Assembly;
extern int Show_Execution;

#endif
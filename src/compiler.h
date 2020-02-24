#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "node.h"
#include "token.h"

/* main function */
Token *scan(FILE*);
Program *parse(Token*);
void codegen(Program*);
Var *find_var(SymbolTable*,char*);
int count_symbol_table(SymbolTable*);
char *get_ident_name(Node*);

/* debug function */
void print_cur_token(Token*);
void print_token();
void print_tree(Node*);
void print_symbol_table(SymbolTable*);
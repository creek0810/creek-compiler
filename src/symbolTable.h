#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include "type.h"

typedef struct Var Var;
typedef struct SymbolTable SymbolTable;

/*
    There is 3 types of symbol_table:
    1. local scope
    2. global scope
    3. parameter scope
*/

/*
    local scope 
    cur_symbol_table
          |
          V
        scope 1 -> scope 2
*/

struct Var {
    Var *next;
    char *name;
    int offset;
    Type *type;
};

struct SymbolTable {
    SymbolTable *prev;
    Var *var;
};

/* function declaration */
SymbolTable *new_symbol_table();
void push_local_table(SymbolTable *new_table);
void pop_local_table();
void add_var_to_symbol_table(SymbolTable* tar_table, char *name, Type *cur_type);
int get_var_offset();
Var *find_var(SymbolTable *symbol_table, char *name);
void init_var_offset();

#endif
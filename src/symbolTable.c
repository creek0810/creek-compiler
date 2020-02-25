#include <string.h>
#include <stdlib.h>
#include "symbolTable.h"

int local_var_offset = 0;
SymbolTable *cur_local_table = NULL;

/* public function */
SymbolTable *new_symbol_table() {
    return calloc(1, sizeof(SymbolTable));
}

void push_local_table(SymbolTable *new_table) {
    new_table->prev = cur_local_table;
    cur_local_table = new_table;
}

void pop_local_table() {
    cur_local_table = cur_local_table->prev;
}



void add_var_to_symbol_table(SymbolTable* tar_table, char *name, Type *cur_type) {
    Var *new_var = calloc(1, sizeof(Var));
    new_var->name = name;
    new_var->type = cur_type;
    new_var->offset = local_var_offset + cur_type->size;
    local_var_offset = new_var->offset;

    new_var->next = tar_table->var;
    tar_table->var = new_var;
}

void init_var_offset() {
    local_var_offset = 0;
}

int get_var_offset() {
    return local_var_offset;
}

Var *find_var(SymbolTable *symbol_table, char *name) {
    if(symbol_table == NULL) {
        return NULL;
    }
    while(symbol_table) {
        Var *cur_var = symbol_table->var;
        while(cur_var) {
            int name_len = strlen(cur_var->name);
            if(name_len == strlen(name) && strncmp(cur_var->name, name, name_len) == 0) {
                return cur_var;
            }
            cur_var = cur_var->next;
        }
        symbol_table = symbol_table->prev;
    }
    return NULL;
}
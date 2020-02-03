#include "compiler.h"

SymbolTable *cur_symbol_table = NULL;
NodeList *function_list = NULL;
NodeList *cur_function_node = NULL;

int var_offset = 0;

/*
    cur_symbol_table
          |
          V
        scope 1 -> scope 2 -> function -> global
*/
/* symbol table function */
SymbolTable *push_symbol_table() {
    SymbolTable *new_table = calloc(1, sizeof(SymbolTable));
    new_table->prev = cur_symbol_table;
    cur_symbol_table = new_table;
    return new_table;
}

void pop_symbol_table() {
    cur_symbol_table = cur_symbol_table->prev;
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

void add_var_to_symbol_table(char *name, Type *cur_type) {
    Var *new_var = calloc(1, sizeof(Var));
    new_var->name = name;
    new_var->type = cur_type;
    int padding = (new_var->offset) % cur_type->aligned;
    new_var->offset = var_offset + padding + cur_type->size;
    var_offset += new_var->offset;

    new_var->next = cur_symbol_table->var;
    cur_symbol_table->var = new_var;
}

Var *add_var_to_varlist(Var *cur_var, char *name, Type *cur_type) {
    Var *new_var = calloc(1, sizeof(Var));
    new_var->name = name;
    new_var->type = cur_type;
    int padding = (new_var->offset) % cur_type->aligned;
    new_var->offset = cur_var->offset + padding + cur_type->size;
    cur_var->next = new_var;
    return new_var;
}

/* help function */
char *get_ident_name(Node *cur_node) {
    cur_node = cur_node->extend.binode.rhs;
    if(cur_node->type == ND_ASSIGN) {
        return cur_node->extend.binode.lhs->extend.name;
    }
    return cur_node->extend.name;
}
void next_token() {
    cur_token = cur_token->next;
}

bool consume_op(char *str) {
    if (cur_token->type == TK_PUNC &&
        strlen(str) == cur_token->len &&
        strncmp(cur_token->str, str, cur_token->len) == 0) {
        cur_token = cur_token->next;
        return true;
    }
    return false;
}

bool consume_keyword(char *str) {
    if( cur_token->type == TK_KEYWORD &&
        cur_token->len == strlen(str) &&
        strncmp(cur_token->str, str, cur_token->len) == 0) {
        cur_token = cur_token->next;
        return true;
    }
    return false;
}

bool consume_ident() {
    if(cur_token->type == TK_IDENT) {
        cur_token = cur_token->next;
        return true;
    }
    return false;
}

/* check */
const int op_len = 6;
const char *op[6] = {"&", "*", "+", "-", "~", "!"};
int is_unary_operator(){
    // & * + - ~ !
    if(cur_token->type == TK_PUNC && cur_token->len == 1) {
        for(int i=0; i<op_len; i++) {
            if(strncmp(op[i], cur_token->str, 1) == 0) {
                return i;
            }
        }
    }
    // return -1 if not a unary operator
    return -1;
}

const int jump_keyword_len = 4;
const char *jump_keyword[4] = {
    "goto", "continue",
    "break", "return"
};
bool is_jump() {
    if(cur_token->type == TK_KEYWORD) {
        for(int i=0; i<jump_keyword_len; i++) {
            if(strlen(jump_keyword[i]) == cur_token->len &&
               strncmp(cur_token->str, jump_keyword[i], cur_token->len) == 0) {
                   return true;
            }
        }
    }
    return false;
}

const int iteration_keyword_len = 3;
const char *iteration_keyword[3] = {
    "do", "while", "for"
};
bool is_iteration() {
    if(cur_token->type == TK_KEYWORD) {
        for(int i=0; i<iteration_keyword_len; i++) {
            if(strlen(iteration_keyword[i]) == cur_token->len &&
               strncmp(cur_token->str, iteration_keyword[i], cur_token->len) == 0) {
                   return true;
            }
        }
    }
    return false;
}

const int selection_keyword_len = 2;
const char *selection_keyword[2] = {
    "if", "switch"
};
bool is_selection() {
    if(cur_token->type == TK_KEYWORD) {
        for(int i=0; i<selection_keyword_len; i++) {
            if(strlen(selection_keyword[i]) == cur_token->len &&
               strncmp(cur_token->str, selection_keyword[i], cur_token->len) == 0) {
                   return true;
            }
        }
    }
    return false;
}

const type_keyword_len = 9;
const char *type_keyword[9] = {
    // "auto", "register", "static", "extern", "typedef",
    "void", "char", "short", "int", "long", "float", "double",
    "signed", "unsigned",
};

bool is_type() {
    /*
                   | <struct-or-union-specifier>
                   | <enum-specifier>
                   | <typedef-name>
    <type-qualifier> ::= const
                   | volatile
    */
   for(int i=0; i<type_keyword_len; i++) {
       if(strlen(type_keyword[i]) == cur_token->len &&
          strncmp(type_keyword[i], cur_token->str, cur_token->len) == 0) {
              return true;
        }
   }
   return false;
}


/* node function */
Node *add_node_int(int val) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = ND_INT;
    new_node->extend.val = val;
    return new_node;
}

Node *add_node_unary(NodeType type, Node *expr) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = type;
    new_node->extend.expr = expr;
    return new_node;
}

Node *add_node_bi_op(NodeType type, Node *lhs, Node *rhs){
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = type;
    new_node->extend.binode.lhs = lhs;
    new_node->extend.binode.rhs = rhs;
    return new_node;
}

Node *add_node_ter_op(Node *condition_expr, Node *if_stmt, Node *else_stmt) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = ND_IF;
    new_node->extend.ternode.condition = condition_expr;
    new_node->extend.ternode.if_stmt = if_stmt;
    new_node->extend.ternode.else_stmt = else_stmt;
    return new_node;
}

Node *add_node_block(NodeList *stmts, SymbolTable *symbol_table) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = ND_BLOCK;
    new_node->extend.blocknode.stmts = stmts;
    new_node->extend.blocknode.symbol_table = symbol_table;
    return new_node;
}

Node *add_node_ident(char *name) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = ND_IDENT;
    new_node->extend.name = name;
    return new_node;
}

Node *add_node_loop(NodeType type, Node *init, Node *cur_cond, Node *cur_stmt, Node *after) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = type;
    new_node->extend.loopnode.init = init;
    new_node->extend.loopnode.condition = cur_cond;
    new_node->extend.loopnode.stmt = cur_stmt;
    new_node->extend.loopnode.after_check = after;
    return new_node;
}

Node *add_node_function(Type *return_type, char *name, Node *stmt, int memory) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = ND_FUNCTION;
    new_node->extend.functionnode.name = name;
    new_node->extend.functionnode.stmt = stmt;
    new_node->extend.functionnode.memory = memory;
    new_node->extend.functionnode.return_type = return_type;
    return new_node;
}


/* parse function declaration */
bool is_func_def();
Node *stmt();
Node *selection_stmt();
Node *jump_stmt();
Node *iteration_stmt(); // ok
Node *compound_stmt(); // ok
Node *declaration();
Node *direct_declarator();
Type *declaration_specifier();
Node *declarator();
Node *init_declarator(); // ok
Node *initializer();
Node *expr_stmt(); // ok
Node *expr();
Node *assign(); // ok
Node *condition(); // ok
Node *logic_or(); // ok
Node *logic_and(); // ok
Node *inclusive_or(); // ok
Node *exclusive_or(); // ok
Node *and_expr(); // ok
Node *equality(); // ok
Node *relational(); // ok
Node *shift(); // ok
Node *additive(); // ok
Node *multiplicative(); // ok
Node *unary();
Node *cast();
Node *postfix();
Node *primary();


/* try to parse function definition */

bool is_func_def() {
    bool flag = true;
    Token *copy_token = cur_token;
    Type *type = declaration_specifier();
    Node *decl = declarator();
    if(consume_op("=") || consume_op(";")) { // is init-declarator
        flag = false;
    }
    cur_token = copy_token;
    return flag;
}


/*
<primary-expression> ::= <identifier> ok
                       | <constant> incomplete
                       | <string>
                       | ( <expression> ) ok
*/
Node *primary() {
    Node *cur_node;
    // type to select action
    switch(cur_token->type) {
        case TK_INT:
        case TK_FLOAT:
        {
            // support different type
            cur_node = add_node_int(atoi(cur_token->str));
            next_token();
            return cur_node;
        }
        case TK_IDENT: {
            cur_node = add_node_ident(cur_token->str);
            next_token();
            return cur_node;
        }
    }
    if(consume_op("(")) {
        cur_node = expr();
        consume_op(")");
        return cur_node;
    }
}

/*
<postfix-expression> ::= <primary-expression> ok
                       | <postfix-expression> [ <expression> ]
                       | <postfix-expression> ( {<assignment-expression>}* )
                       | <postfix-expression> . <identifier>
                       | <postfix-expression> -> <identifier>
                       | <postfix-expression> ++
                       | <postfix-expression> --
*/
Node *postfix() {
    Node *cur_node = primary();
    if(consume_op("(")) {
        // suppose no arg
        // TODO: support arg
        cur_node = add_node_bi_op(ND_CALL, cur_node, NULL);
        consume_op(")");
        return cur_node;
    }
    return cur_node;
}

/*
<cast-expression> ::= <unary-expression> ok
                    | ( <type-name> ) <cast-expression>
*/
Node *cast() {
    return unary();
}

/*
<unary-expression> ::= <postfix-expression> ok
                     | ++ <unary-expression>
                     | -- <unary-expression>
                     | <unary-operator> <cast-expression> ok
                     | sizeof <unary-expression>
                     | sizeof <type-name>
*/
Node *unary() {
    int op_idx;
    if((op_idx = is_unary_operator()) != -1) {
        // TODO: * / not finished
        next_token();
        Node *node = cast();
        // 0: '&', 1: '*', 2: '+', 3: '-', 4: '~', 5: '!'
        switch (op_idx) {
        case 0:
        case 1:
            printf("& * is not implemented\n");
            break;
        case 2:
            return add_node_bi_op(ND_ADD, add_node_int(0), cast());
        case 3:
            return add_node_bi_op(ND_SUB, add_node_int(0), cast());
        case 4:
            return add_node_unary(ND_BIT_NOT, cast());
        case 5:
            return add_node_unary(ND_LOGIC_NOT, cast());
        default:
            printf("parse unary error\n");
            break;
        }
    }
    return postfix();
}

/* codegen ok
<multiplicative-expression> ::= <cast-expression> ok
                              | <multiplicative-expression> * <cast-expression> ok
                              | <multiplicative-expression> / <cast-expression> ok
                              | <multiplicative-expression> % <cast-expression> ok
*/
Node *multiplicative() {
    Node *cur_node = cast();
    while(true) {
        if(consume_op("*")) {
            cur_node = add_node_bi_op(ND_MUL, cur_node, cast());
        } else if(consume_op("/")) {
            cur_node = add_node_bi_op(ND_DIV, cur_node, cast());
        } else if(consume_op("%")) {
            cur_node = add_node_bi_op(ND_MOD, cur_node, cast());
        } else {
            break;
        }
    }
    return cur_node;
}

/* codegen ok
<additive-expression> ::= <multiplicative-expression> ok
                        | <additive-expression> + <multiplicative-expression> ok
                        | <additive-expression> - <multiplicative-expression> ok
*/
Node *additive() {
    Node *cur_node = multiplicative();
    while(true) {
        if(consume_op("+")) {
            cur_node = add_node_bi_op(ND_ADD, cur_node, multiplicative());
        } else if(consume_op("-")){
            cur_node = add_node_bi_op(ND_SUB, cur_node, multiplicative());
        } else {
            break;
        }
    }
    return cur_node;
}

/* codegen ok
<shift-expression> ::= <additive-expression> ok
                     | <shift-expression> << <additive-expression> ok
                     | <shift-expression> >> <additive-expression> ok
*/
Node *shift() {
    Node *cur_node = additive();
    while(true) {
        if(consume_op("<<")) {
            cur_node = add_node_bi_op(ND_LSHIFT, cur_node, additive());
        } else if(consume_op(">>")){
            cur_node = add_node_bi_op(ND_RSHIFT, cur_node, additive());
        } else {
            break;
        }
    }
    return cur_node;
}

/* codegen ok
<relational-expression> ::= <shift-expression> ok
                          | <relational-expression> < <shift-expression> ok
                          | <relational-expression> > <shift-expression> ok
                          | <relational-expression> <= <shift-expression> ok
                          | <relational-expression> >= <shift-expression> ok
*/
Node *relational() {
    Node *cur_node = shift();
    while(true) {
        if(consume_op("<")) {
            cur_node = add_node_bi_op(ND_LT, cur_node, shift());
        } else if(consume_op(">")){
            cur_node = add_node_bi_op(ND_LT, shift(), cur_node);
        } else if(consume_op("<=")){
            cur_node = add_node_bi_op(ND_LE, cur_node, shift());
        } else if(consume_op(">=")){
            cur_node = add_node_bi_op(ND_LE, shift(), cur_node);
        } else {
            break;
        }
    }
    return cur_node;
}

/* codegen ok
<equality-expression> ::= <relational-expression> ok
                        | <equality-expression> == <relational-expression> ok
                        | <equality-expression> != <relational-expression> ok
*/
Node *equality() {
    Node *cur_node = relational();
    while(true) {
        if(consume_op("==")) {
            cur_node = add_node_bi_op(ND_EQ, cur_node, relational());
        } else if(consume_op("!=")){
            cur_node = add_node_bi_op(ND_NE, cur_node, relational());
        } else {
            break;
        }
    }
    return cur_node;
}

/* codegen ok
<and-expression> ::= <equality-expression> ok
                   | <and-expression> & <equality-expression> ok
*/
Node *and_expr() {
    Node *cur_node = equality();
    while(consume_op("&")) {
        cur_node = add_node_bi_op(ND_BIT_AND, cur_node, equality());
    }
    return cur_node;
}

/* codegen ok
<exclusive-or-expression> ::= <and-expression> ok
                            | <exclusive-or-expression> ^ <and-expression> ok
*/
Node *exclusive_or() {
    Node *cur_node = and_expr();
    while(consume_op("^")) {
        cur_node = add_node_bi_op(ND_BIT_XOR, cur_node, and_expr());
    }
    return cur_node;
}

/* codegen ok
<inclusive-or-expression> ::= <exclusive-or-expression> ok
                            | <inclusive-or-expression> | <exclusive-or-expression> ok
*/
Node *inclusive_or() {
    Node *cur_node = exclusive_or();
    while(consume_op("|")) {
        cur_node = add_node_bi_op(ND_BIT_OR, cur_node, exclusive_or());
    }
    return cur_node;
}

/* codegen ok
<logical-and-expression> ::= <inclusive-or-expression> ok
                           | <logical-and-expression> && <inclusive-or-expression> ok
*/
Node *logic_and() {
    Node *cur_node = inclusive_or();
    while(consume_op("&&")) {
        cur_node = add_node_bi_op(ND_LOGIC_AND, cur_node, inclusive_or());
    }
    return cur_node;
}

/* codegen ok
<logical-or-expression> ::= <logical-and-expression> ok
                          | <logical-or-expression> || <logical-and-expression> ok
*/
Node *logic_or() {
    Node *cur_node = logic_and();
    while(consume_op("||")) {
        cur_node = add_node_bi_op(ND_LOGIC_OR, cur_node, logic_and());
    }
    return cur_node;
}

/* codegen ok
<conditional-expression> ::= <logical-or-expression> ok
                           | <logical-or-expression> ? <expression> : <conditional-expression> ok
*/
Node *condition() {
    Node *cur_node = logic_or();
    if(consume_op("?")) {
        Node *if_stmt = expr();
        consume_op(":");
        Node *else_stmt = condition();
        cur_node = add_node_ter_op(cur_node, if_stmt, else_stmt);
    }
    return cur_node;
}

/* codegen ok
<assignment-expression> ::= <conditional-expression> ok
                          | <unary-expression> <assignment-operator> <assignment-expression> ok
*/
Node *assign() {
    Node *cur_node = condition();
    if(consume_op("=")) {
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, assign());
        return cur_node;
    }
    if(consume_op("*=")) {
        Node *calc = add_node_bi_op(ND_MUL, cur_node, assign());
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("/=")) {
        Node *calc = add_node_bi_op(ND_DIV, cur_node, assign());
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("%=")) {
        Node *calc = add_node_bi_op(ND_MOD, cur_node, assign());
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("+=")) {
        Node *calc = add_node_bi_op(ND_ADD, cur_node, assign());
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("-=")) {
        Node *calc = add_node_bi_op(ND_SUB, cur_node, assign());
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("<<=")) {
        Node *calc = add_node_bi_op(ND_LSHIFT, cur_node, assign());
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op(">>=")) {
        Node *calc = add_node_bi_op(ND_RSHIFT, cur_node, assign());
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("&=")) {
        Node *calc = add_node_bi_op(ND_BIT_AND, cur_node, assign());
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("^=")) {
        Node *calc = add_node_bi_op(ND_BIT_XOR, cur_node, assign());
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("|=")) {
        Node *calc = add_node_bi_op(ND_BIT_OR, cur_node, assign());
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    return cur_node;
}

/*
<expression> ::= <assignment-expression> ok
               | <expression> , <assignment-expression>
*/
Node *expr() {
    return assign();
}

/* <expression-statement> ::= {<expression>}? ;  ok */
Node *expr_stmt() {
    while(true) {
        if(consume_op(";")) {
            continue;
        }
        Node *cur_node = expr();
        consume_op(";");
        return cur_node;
    }
    return NULL;
}

/*
<parameter-declaration> ::= {<declaration-specifier>}+ <declarator>
                          | {<declaration-specifier>}+ <abstract-declarator>
                          | {<declaration-specifier>}+
*/
Var *parameter_declaration() {

}

/*
<parameter-list> ::= <parameter-declaration> ok
                   | <parameter-list> , <parameter-declaration>
*/
Var *parameter_list() {
    Var *cur_node = parameter_declaration();
    while(cur_node) {
        cur_node = parameter_declaration();
    }
    return cur_node;
}


/*
<parameter-type-list> ::= <parameter-list> ok
                        | <parameter-list> , ...
*/
Var *parameter_type_list() {
    return parameter_list();
}

/* <direct-declarator> ::= <identifier> ok
                      | ( <declarator> ) ok
                      | <direct-declarator> [ {<constant-expression>}? ]
                      | <direct-declarator> ( <parameter-type-list> )
                      | <direct-declarator> ( {<identifier>}* )
*/
Node *direct_declarator() {
    Node *node;
    // next step
    if(cur_token->type == TK_IDENT) {
        node = add_node_ident(cur_token->str);
        next_token();
    } else if(consume_op("(")) {
        node = declarator();
        consume_op(")");
    }
    // left recursive
    while(true) {
        if(consume_op("(")) {
            // suppose no parameter only ()
            // TODO: support parameter
            consume_op(")");
            return node;
        } else if(consume_op("[")) {

        } else {
            return node;
        }
    }
}

/*
<declaration-specifier> ::= <storage-class-specifier>
                          | <type-specifier>
                          | <type-qualifier>
<type-specifier> ::= void
                   | char
                   | short
                   | int
                   | long
                   | float
                   | double
                   | signed
                   | unsigned
                   | <struct-or-union-specifier>
                   | <enum-specifier>
                   | <typedef-name>

<storage-class-specifier> ::= auto
                            | register
                            | static
                            | extern
                            | typedef
<type-qualifier> ::= const
                   | volatile
*/
Type *declaration_specifier() {
    // TODO: support more type
    if(is_type()) {
        if(consume_keyword("int")) {
            return &INT_TYPE;
        }
        if(consume_keyword("char")) {
            return &CHAR_TYPE;
        }
    }
    return NULL;
}

/*
<initializer> ::= <assignment-expression>
                | { <initializer-list> }
                | { <initializer-list> , }
*/
Node *initializer() {
    return assign();
}

/* <declarator> ::= {<pointer>}? <direct-declarator> */
Node *declarator() {
    return direct_declarator();
}

/* <declaration> ::=  {<declaration-specifier>}+ {<init-declarator>}* ;
   <init-declarator> ::= <declarator> ok
                       | <declarator> = <initializer> ok
*/
Node *declaration() {
    // TODO: support init several variable
    Type *type = declaration_specifier();
    if(!type) {
        // printf("expected type");
        return NULL;
    }
    Node *node = NULL;
    while(!consume_op(";")) {
        Node *cur_ident = declarator();
        // add to symbol table
        add_var_to_symbol_table(cur_ident->extend.name, type);
        if(consume_op("=")) {
            node = add_node_bi_op(ND_ASSIGN, cur_ident, initializer());
        }
    }
    return node;
}

/* codegen ok
<compound-statement> ::= { {<declaration>}* {<statement>}*  ok}
*/
Node *compound_stmt() {
    /* warning: "{" has been consumed at stmt stage */
    NodeList *node_list = NULL;
    NodeList *cur_node;

    // init symbol_table;
    SymbolTable *symbol_table = push_symbol_table();
    while(!consume_op("}")) {
        cur_symbol_table = symbol_table;
        Node *decl_stmt = NULL;
        // may be declaration or stmt
        if(!(decl_stmt = declaration())) {
            decl_stmt = stmt();
        }
        // append to node_list
        if(!node_list) {
            node_list = calloc(1, sizeof(NodeList));
            cur_node = node_list;
            cur_node->tree = decl_stmt;
        } else {
            NodeList *new_node = calloc(1, sizeof(NodeList));
            new_node->tree = decl_stmt;
            cur_node->next = new_node;
            cur_node = new_node;
        }
    }
    Node *block_node = add_node_block(node_list, symbol_table);
    pop_symbol_table();
    return block_node;
}

/* codegen ok
<iteration-statement> ::= while ( <expression> ) <statement> ok ok
                        | do <statement> while ( <expression> ) ;  ok ok
                        | for ( {<expression>}? ; {<expression>}? ; {<expression>}? ) <statement> ok ok
*/
Node *iteration_stmt() {
    if(consume_keyword("while")) {
        consume_op("(");
        Node *cur_condition = expr();
        consume_op(")");
        Node *cur_stmt = stmt();
        return add_node_loop(ND_LOOP, NULL, cur_condition, cur_stmt, NULL);
    }
    if(consume_keyword("for")) {
        Node *init = NULL, *cur_cond = NULL, *after = NULL;
        consume_op("(");
        if(!consume_op(";")) {
            init = expr();
            consume_op(";");
        }

        if(!consume_op(";")) {
            cur_cond = expr();
            consume_op(";");
        }

        if(!consume_op(")")) {
            after = expr();
            consume_op(")");
        }
        return add_node_loop(ND_LOOP, init, cur_cond, stmt(), after);
    }
    if(consume_keyword("do")) {
        Node *cur_stmt = stmt();
        consume_keyword("while");
        consume_op("(");
        Node *cur_cond = expr();
        consume_op(")");
        consume_op(";");
        return add_node_loop(ND_DO_LOOP, NULL, cur_cond, cur_stmt, NULL);
    }
}


/* <jump-statement> ::= goto <identifier> ;
                   | continue ; ok ok
                   | break ; ok ok
                   | return {<expression>}? ; ok ok
*/
Node *jump_stmt() {
    if(consume_keyword("goto")) {
        consume_op(";");
    } else if (consume_keyword("continue")) {
        consume_op(";");
        return add_node_unary(ND_CONTINUE, NULL);
    } else if(consume_keyword("break")) {
        consume_op(";");
        return add_node_unary(ND_BREAK, NULL);
    } else if(consume_keyword("return")) {
        if(consume_op(";")) {
            return add_node_unary(ND_RETURN, NULL);
        }
        Node *cur_node = add_node_unary(ND_RETURN, expr());
        consume_op(";");
        return cur_node;
    }
}

/*
<selection-statement> ::= if ( <expression> ) <statement> ok
                        | if ( <expression> ) <statement> else <statement> ok
                        | switch ( <expression> ) <statement>
*/
Node *selection_stmt() {
    if(consume_keyword("if")) {
        consume_op("(");
        Node *cond = expr();
        consume_op(")");
        Node *if_true = stmt();
        if(consume_keyword("else")) {
            return add_node_ter_op(cond, if_true, stmt());
        }
        return add_node_ter_op(cond, if_true, NULL);
    }
    if(consume_keyword("switch")) {
        // TODO: support switch
    }
    return NULL;
}

/*
<statement> ::= <labeled-statement>
              | <expression-statement> ok
              | <compound-statement> ok
              | <selection-statement> ok
              | <iteration-statement> ok
              | <jump-statement> ok
*/

Node *stmt() {
    Node *cur_node = NULL;
    if(consume_op("{")) {
        return compound_stmt();
    } else if(is_jump()) {
        return jump_stmt();
    } else if(is_selection()) {
        return selection_stmt();
    } else if(is_iteration()) {
        return iteration_stmt();
    }
    return expr_stmt();
}

/* codegen ok
<function-definition> ::= {<declaration-specifier>}* <declarator> {<declaration>}* <compound-statement> ok
*/
Node *function_definition() {
    // TODO: support declaration-specifier
    Type *return_type = declaration_specifier();
    Node *node = declarator();
    if(consume_op("{")) {
        // init function symbol table
        cur_symbol_table = NULL;
        node = add_node_function(return_type, node->extend.name, compound_stmt(), var_offset);
    }
    return node;
}

/*
<external-declaration> ::= <function-definition> ok
                         | <declaration> ok
*/
Node *external_declaration() {
    if(is_func_def()) {
        return function_definition();
    }
    return declaration();
}

void parse() {
    /*
        <translation-unit> ::= {<external-declaration>}* ok
    */
    while(cur_token->type != TK_EOF) {
        Node *cur_tree_node = external_declaration();
        NodeList *new_node = calloc(1, sizeof(NodeList));
        new_node->tree = cur_tree_node;
        if(function_list) {
            cur_function_node->next = new_node;
            cur_function_node = new_node;
        } else {
            function_list = new_node;
            cur_function_node = new_node;
        }
    }
}
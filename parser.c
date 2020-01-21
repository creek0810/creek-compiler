#include "compiler.h"

SymbolTable *cur_symbol_table = NULL;
SymbolTable *symbol_table_head = NULL;

int var_offset = 0;

/*
   (SymbolTable.inner)
                    <-
function -> scope 1 -> scope 3
         <-   | (SymbolTableList.next)
              V
            scope 2 -> scope 5
                    <-
*/
/* symbol table function */
SymbolTable *add_symbol_table() {
    SymbolTable *new_table = calloc(1, sizeof(SymbolTable));
    if(cur_symbol_table) {
        SymbolTableList *new_table_list = calloc(1, sizeof(SymbolTableList));
        new_table_list->table = new_table;
        new_table_list->next = cur_symbol_table->inner;
        cur_symbol_table->inner = new_table_list;
        new_table->prev = cur_symbol_table;
    } else {
        cur_symbol_table = new_table;
        symbol_table_head = cur_symbol_table;
    }
    return new_table;
}

Var *find_var(SymbolTable *symbol_table, char *name) {
    if(symbol_table == NULL) {
        return NULL;
    }
    Var *cur_var = symbol_table->var;
    while(cur_var) {
        int name_len = strlen(cur_var->name);
        if(name_len == strlen(name) && strncmp(cur_var->name, name, name_len) == 0) {
            return cur_var;
        }
        cur_var = cur_var->next;
    }
    return find_var(symbol_table->prev, name);
}

void add_var_to_symbol_table(char *name) {
    Var *new_var = calloc(1, sizeof(Var));
    new_var->name = name;
    new_var->offset = var_offset++;
    new_var->next = cur_symbol_table->var;
    cur_symbol_table->var = new_var;
    cur_symbol_table->cnt += 1;
}

int count_symbol_table(SymbolTable *cur_table) {
    int cnt = 0;
    if(!cur_table) {
        return 0;
    }
    // add cur
    cnt += cur_table->cnt;
    SymbolTableList *cur_it_table = cur_table->inner;
    while(cur_it_table) {
        cnt += count_symbol_table(cur_it_table->table);
        cur_it_table = cur_it_table->next;
    }
    return cnt;
}

/* help function */
void next_token() {
    cur_token = cur_token->next;
}

const int jump_keyword_len = 4;
const char *jump_keyword[4] = {
    "goto", "continue",
    "break", "return"
};

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

/* parse function */
Node *stmt();
Node *jump_stmt();
Node *compound_stmt();
Node *declaration();
Node *direct_declarator();
Node *declaration_specifier();
Node *declarator();
Node *init_declarator();
Node *expr_stmt();
Node *expr();
Node *assign();
Node *condition();
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


/* definition */

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
        case TK_CONSTANT: {
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
        } else if(consume_op("\%")) {
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
    while(true) {
        if(consume_op("&")) {
            cur_node = add_node_bi_op(ND_BIT_AND, cur_node, equality());
        } else {
            break;
        }
    }
    return cur_node;
}

/* codegen ok
<exclusive-or-expression> ::= <and-expression> ok
                            | <exclusive-or-expression> ^ <and-expression> ok
*/
Node *exclusive_or() {
    Node *cur_node = and_expr();
    while(true) {
        if(consume_op("^")) {
            cur_node = add_node_bi_op(ND_BIT_XOR, cur_node, and_expr());
        } else {
            break;
        }
    }
    return cur_node;
}

/* codegen ok
<inclusive-or-expression> ::= <exclusive-or-expression> ok
                            | <inclusive-or-expression> | <exclusive-or-expression> ok
*/
Node *inclusive_or() {
    Node *cur_node = exclusive_or();
    while(true) {
        if(consume_op("|")) {
            cur_node = add_node_bi_op(ND_BIT_OR, cur_node, exclusive_or());
        } else {
            break;
        }
    }
    return cur_node;
}

/* codegen ok
<logical-and-expression> ::= <inclusive-or-expression> ok
                           | <logical-and-expression> && <inclusive-or-expression> ok
*/
Node *logic_and() {
    Node *cur_node = inclusive_or();
    while(true) {
        if(consume_op("&&")) {
            cur_node = add_node_bi_op(ND_LOGIC_AND, cur_node, inclusive_or());
        } else {
            break;
        }
    }
    return cur_node;
}

/* codegen ok
<logical-or-expression> ::= <logical-and-expression> ok
                          | <logical-or-expression> || <logical-and-expression> ok
*/
Node *logic_or() {
    Node *cur_node = logic_and();
    while(true) {
        if(consume_op("||")) {
            cur_node = add_node_bi_op(ND_LOGIC_OR, cur_node, logic_and());
        } else {
            break;
        }
    }
    return cur_node;
}

/*
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

/*
<assignment-expression> ::= <conditional-expression> ok
                          | <unary-expression> <assignment-operator> <assignment-expression>
*/
Node *assign() {
    Node *cur_node = condition();
    // TODO: support another assign op
    if(consume_op("=")) {
        cur_node = add_node_bi_op(ND_ASSIGN, cur_node, assign());
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

/* <direct-declarator> ::= <identifier> ok
                      | ( <declarator> )
                      | <direct-declarator> [ {<constant-expression>}? ]
                      | <direct-declarator> ( <parameter-type-list> )
                      | <direct-declarator> ( {<identifier>}* )
*/
Node *direct_declarator() {
    if(cur_token->type == TK_IDENT) {
        Node *node = add_node_ident(cur_token->str);
        next_token();
        return node;
    }
}

/*
<declaration-specifier> ::= <storage-class-specifier>
                          | <type-specifier>
                          | <type-qualifier>
*/
Node *declaration_specifier() {
    /*
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
    */
   // TODO: support another type

}

/* <declarator> ::= {<pointer>}? <direct-declarator> */
Node *declarator() {

    return direct_declarator();
}

/*
<init-declarator> ::= <declarator> ok
                    | <declarator> = <initializer>
*/
Node *init_declarator() {

    return declarator();
}

/* <declaration> ::=  {<declaration-specifier>}+ {<init-declarator>}* ; */
Node *declaration() {
    Node *node = NULL;
    while(!consume_op(";")) {
        // hack
        //TODO: finish declaration-specifier
        if(strncmp(cur_token->str, "int", 3) != 0)
            return node;
        next_token();
        node = init_declarator();
    }
    return node;
}

/*
<compound-statement> ::= { {<declaration>}* {<statement>}*  ok}
*/
Node *compound_stmt() {
    /* warning: "{" has been consumed at stmt stage */
    NodeList *node_list = NULL;
    NodeList *cur_node;

    // init symbol_table;
    SymbolTable *symbol_table = add_symbol_table();
    while(!consume_op("}")) {
        Node *decl_stmt = NULL;
        // may be declaration or stmt
        if((decl_stmt = declaration()) != NULL) {
            // add to symbol table
            cur_symbol_table = symbol_table;
            add_var_to_symbol_table(decl_stmt->extend.name);
            // establish declaration node;
            decl_stmt = add_node_bi_op(ND_DECLARE, decl_stmt, NULL);
        } else {
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
    return add_node_block(node_list, symbol_table);
}

/* <jump-statement> ::= goto <identifier> ;
                   | continue ; ok
                   | break ; ok
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
            return NULL;
        }
        Node *cur_node = add_node_unary(ND_RETURN, expr());
        consume_op(";");
        return cur_node;
    }
}

/*
<statement> ::= <labeled-statement>
              | <expression-statement> ok
              | <compound-statement> ok
              | <selection-statement>
              | <iteration-statement>
              | <jump-statement> ok
*/

Node *stmt() {
    if(consume_op("{")) {
        return compound_stmt();
    } else if(is_jump()) {
        return jump_stmt();
    }
    return expr_stmt();
}

Node *parse() {
    return stmt();
}
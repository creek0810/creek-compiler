#include "compiler.h"

Token *cur_token;
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

void add_var_to_symbol_table(SymbolTable* tar_table, char *name, Type *cur_type) {
    Var *new_var = calloc(1, sizeof(Var));
    new_var->name = name;
    new_var->type = cur_type;
    int padding = (new_var->offset) % cur_type->aligned;
    new_var->offset = var_offset + padding + cur_type->size;
    var_offset += new_var->offset;

    new_var->next = tar_table->var;
    tar_table->var = new_var;
}

/* help function */
void append_node_list(NodeList *vec, Node *new_node) {
    if(vec->tree == NULL) {
        vec->tree = new_node;
    } else {
        // alloc new node list
        NodeList *new_node_list = calloc(1, sizeof(NodeList));
        new_node_list->tree = new_node;
        // find the latest node
        while(vec->next != NULL) vec = vec->next;
        vec->next = new_node_list;
    }
}

NodeList* prepend_node_list(NodeList *vec, Node *new_node) {
    NodeList *new_node_list = calloc(1, sizeof(NodeList));
    new_node_list->tree = new_node;
    new_node_list->next = vec;
    return new_node_list;
}

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


bool next_is(char *str) {
    if (strlen(str) == cur_token->len &&
        strncmp(cur_token->str, str, cur_token->len) == 0) {
        cur_token = cur_token->next;
        return true;
    }
    return false;
}
char *consume_ident() {
    if(cur_token->type == TK_IDENT) {
        char *tmp = cur_token->str;
        next_token();
        return tmp;
    }
    printf("should be ident!\n");
    exit(1);
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

bool consume_keyword(TokenType type) {
    if( cur_token->type == type) {
        cur_token = cur_token->next;
        return true;
    }
    return false;
}


/* parse function declaration */
bool is_func_def();
NodeList *translation_unit();
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
            cur_node = new_int_node(atoi(cur_token->str));
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
    // return cur_node;
}

/*
<postfix-expression> ::= <primary-expression> ok
                       | <postfix-expression> [ <expression> ]
                       | <postfix-expression> ( {<assignment-expression>}* )
                       | <postfix-expression> . <identifier>
                       | <postfix-expression> -> <identifier>
                       | <postfix-expression> ++ ok
                       | <postfix-expression> -- ok
*/
Node *postfix() {
    Node *cur_node = primary();
    // function call
    if(consume_op("(")) {
        NodeList *arg_list = NULL;
        // read arg
        if(!consume_op(")")) {
            arg_list = prepend_node_list(arg_list, assign());
            while(consume_op(",")) {
                arg_list = prepend_node_list(arg_list, assign());
            }
            consume_op(")");
        }
        return new_call_node(cur_node, arg_list);
    }
    if(consume_op("++")) {
        cur_node = new_unary_node(ND_POST_INC, cur_node);
    }
    if(consume_op("--")) {
        cur_node = new_unary_node(ND_POST_DEC, cur_node);
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
                     | ++ <unary-expression> ok
                     | -- <unary-expression> ok
                     | <unary-operator> <cast-expression> ok
                     | sizeof <unary-expression>
                     | sizeof <type-name>
<unary-operator> ::= & | * | + | - | ~ | !
*/
Node *unary() {
    if(consume_op("&")) {
        return new_unary_node(ND_ADDR, cast());
    }
    if(consume_op("*")) {
        return new_unary_node(ND_DEREF, cast());
    }
    if(consume_op("+")) {
        return new_binary_node(ND_ADD, new_int_node(0), cast());
    }
    if(consume_op("-")) {
        return new_binary_node(ND_SUB, new_int_node(0), cast());
    }
    if(consume_op("~")) {
        return new_unary_node(ND_BIT_NOT, cast());
    }
    if(consume_op("!")) {
        return new_unary_node(ND_LOGIC_NOT, cast());
    }
    if(consume_op("++")) {
        return new_unary_node(ND_PRE_INC, unary());
    }
    if(consume_op("--")) {
        return new_unary_node(ND_PRE_DEC, unary());
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
            cur_node = new_binary_node(ND_MUL, cur_node, cast());
        } else if(consume_op("/")) {
            cur_node = new_binary_node(ND_DIV, cur_node, cast());
        } else if(consume_op("%")) {
            cur_node = new_binary_node(ND_MOD, cur_node, cast());
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
            cur_node = new_binary_node(ND_ADD, cur_node, multiplicative());
        } else if(consume_op("-")){
            cur_node = new_binary_node(ND_SUB, cur_node, multiplicative());
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
            cur_node = new_binary_node(ND_LSHIFT, cur_node, additive());
        } else if(consume_op(">>")){
            cur_node = new_binary_node(ND_RSHIFT, cur_node, additive());
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
            cur_node = new_binary_node(ND_LT, cur_node, shift());
        } else if(consume_op(">")){
            cur_node = new_binary_node(ND_LT, shift(), cur_node);
        } else if(consume_op("<=")){
            cur_node = new_binary_node(ND_LE, cur_node, shift());
        } else if(consume_op(">=")){
            cur_node = new_binary_node(ND_LE, shift(), cur_node);
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
            cur_node = new_binary_node(ND_EQ, cur_node, relational());
        } else if(consume_op("!=")){
            cur_node = new_binary_node(ND_NE, cur_node, relational());
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
        cur_node = new_binary_node(ND_BIT_AND, cur_node, equality());
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
        cur_node = new_binary_node(ND_BIT_XOR, cur_node, and_expr());
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
        cur_node = new_binary_node(ND_BIT_OR, cur_node, exclusive_or());
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
        cur_node = new_binary_node(ND_LOGIC_AND, cur_node, inclusive_or());
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
        cur_node = new_binary_node(ND_LOGIC_OR, cur_node, logic_and());
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
        cur_node = new_ternary_node(cur_node, if_stmt, else_stmt);
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
        cur_node = new_binary_node(ND_ASSIGN, cur_node, assign());
        return cur_node;
    }
    if(consume_op("*=")) {
        Node *calc = new_binary_node(ND_MUL, cur_node, assign());
        cur_node = new_binary_node(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("/=")) {
        Node *calc = new_binary_node(ND_DIV, cur_node, assign());
        cur_node = new_binary_node(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("%=")) {
        Node *calc = new_binary_node(ND_MOD, cur_node, assign());
        cur_node = new_binary_node(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("+=")) {
        Node *calc = new_binary_node(ND_ADD, cur_node, assign());
        cur_node = new_binary_node(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("-=")) {
        Node *calc = new_binary_node(ND_SUB, cur_node, assign());
        cur_node = new_binary_node(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("<<=")) {
        Node *calc = new_binary_node(ND_LSHIFT, cur_node, assign());
        cur_node = new_binary_node(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op(">>=")) {
        Node *calc = new_binary_node(ND_RSHIFT, cur_node, assign());
        cur_node = new_binary_node(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("&=")) {
        Node *calc = new_binary_node(ND_BIT_AND, cur_node, assign());
        cur_node = new_binary_node(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("^=")) {
        Node *calc = new_binary_node(ND_BIT_XOR, cur_node, assign());
        cur_node = new_binary_node(ND_ASSIGN, cur_node, calc);
        return cur_node;
    }
    if(consume_op("|=")) {
        Node *calc = new_binary_node(ND_BIT_OR, cur_node, assign());
        cur_node = new_binary_node(ND_ASSIGN, cur_node, calc);
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
    // void(1) char(2) short(4) int(8) float(16)
    // double(32) signed(64) unsigned(128)
    // TODO: deal with long(it may be long long)
    long long type_bit_set = 0;
    while(true) {
        TokenType cur_type = cur_token->type;
        bool is_type = true;
        switch(cur_type) {
            case TK_KW_VOID:
                type_bit_set += 1;
                break;
            case TK_KW_CHAR:
                type_bit_set += 2;
                break;
            case TK_KW_SHORT:
                type_bit_set += 4;
                break;
            case TK_KW_INT:
                type_bit_set += 8;
                break;
            case TK_KW_FLOAT:
                type_bit_set += 16;
                break;
            case TK_KW_DOUBLE:
                type_bit_set += 32;
                break;
            case TK_KW_SIGNED:
                type_bit_set += 64;
                break;
            case TK_KW_UNSIGNED:
                type_bit_set += 128;
                break;
            default:
                is_type = false;
                break;
        }
        if(is_type) next_token();
        else break;
    }
    switch(type_bit_set) {
        case 2:
            return &CHAR_TYPE;
        case 8:
        case 64:
        case 72:
            return &INT_TYPE;
        default:
            // printf("unknown type %lld\n", type_bit_set);
            return NULL;
    }
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
        add_var_to_symbol_table(cur_symbol_table, cur_ident->extend.name, type);
        if(consume_op("=")) {
            node = new_binary_node(ND_ASSIGN, cur_ident, initializer());
        }
    }
    return node;
}

/* codegen ok
<compound-statement> ::= { {<declaration>}* {<statement>}*  ok}
*/
Node *compound_stmt() {
    consume_op("{");
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
    if(consume_keyword(TK_KW_WHILE)) {
        consume_op("(");
        Node *cur_condition = expr();
        consume_op(")");
        Node *cur_stmt = stmt();
        return new_loop_node(ND_LOOP, NULL, cur_condition, cur_stmt, NULL);
    }
    if(consume_keyword(TK_KW_FOR)) {
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
        return new_loop_node(ND_LOOP, init, cur_cond, stmt(), after);
    }
    if(consume_keyword(TK_KW_DO)) {
        Node *cur_stmt = stmt();
        consume_keyword(TK_KW_WHILE);
        consume_op("(");
        Node *cur_cond = expr();
        consume_op(")");
        consume_op(";");
        return new_loop_node(ND_DO_LOOP, NULL, cur_cond, cur_stmt, NULL);
    }
}


/* <jump-statement> ::= goto <identifier> ;
                   | continue ; ok ok
                   | break ; ok ok
                   | return {<expression>}? ; ok ok
*/
Node *jump_stmt() {
    if(consume_keyword(TK_KW_GOTO)) {
        consume_op(";");
    } else if (consume_keyword(TK_KW_CONTINUE)) {
        consume_op(";");
        return new_unary_node(ND_CONTINUE, NULL);
    } else if(consume_keyword(TK_KW_BREAK)) {
        consume_op(";");
        return new_unary_node(ND_BREAK, NULL);
    } else if(consume_keyword(TK_KW_RETURN)) {
        if(consume_op(";")) {
            return new_unary_node(ND_RETURN, NULL);
        }
        Node *cur_node = new_unary_node(ND_RETURN, expr());
        consume_op(";");
        return cur_node;
    }
}

/*
selection-statement ::= "if" "(" expression ")" statement ("else" statement)?
                      | "switch" "(" expression ")" statement
*/
Node *selection_stmt() {
    if(consume_keyword(TK_KW_IF)) {
        consume_op("(");
        Node *cond = expr();
        consume_op(")");
        Node *if_true = stmt();
        Node *if_false = consume_keyword(TK_KW_ELSE) ? stmt() : NULL;
        return new_ternary_node(cond, if_true, if_false);
    }
    if(consume_keyword(TK_KW_SWITCH)) {
        // TODO: support switch
    }
    return NULL;
}

/*
statement ::= labeled-statement
            | expression-statement ok
            | compound-statement ok
            | selection-statement ok
            | iteration-statement ok
            | jump-statement ok
*/
Node *stmt() {
    TokenType cur_type = cur_token->type;
    switch(cur_type) {
        // selection stmt
        case TK_KW_IF:
        case TK_KW_CASE:
            return selection_stmt();
        // iteration stmt
        case TK_KW_WHILE:
        case TK_KW_DO:
        case TK_KW_FOR:
            return iteration_stmt();
        // jmp stmt
        case TK_KW_GOTO:
        case TK_KW_CONTINUE:
        case TK_KW_BREAK:
        case TK_KW_RETURN:
            return jump_stmt();
        default: {
            // comp stmt
            if(next_is("{")) return compound_stmt();
            // expr stmt
            else return expr_stmt();
        }
    }
}

/* codegen ok
function-definition ::=
    declaration-specifiers declarator (declaration)* compound-statement
*/
Node *function_definition() {
    /*
     simplify the rule to
     <function-definition> ::=
       {<declaration-specifier>}* <identifier> '(' <arg-list> ')' <compound-statement>
     <arg-list> ::=
        <declaration-specifier> <identifier> {',' <declaration-specifier> <identifier}*
    */
    Type *return_type = declaration_specifier();
    // get function name
    char *name = consume_ident();
    // get arg
    SymbolTable *arg_table = calloc(1, sizeof(SymbolTable));
    NodeList *args = NULL;
    consume_op("(");
    if(!consume_op(")")) {
        args = calloc(1, sizeof(NodeList));
        // store the arg reverse
        Type *cur_type = declaration_specifier();
        char *name = consume_ident();
        append_node_list(args, new_decl_node(cur_type, name));
        add_var_to_symbol_table(arg_table, name, cur_type);

        while(consume_op(",")) {
            Type *cur_type = declaration_specifier();
            char *name = consume_ident();
            append_node_list(args, new_decl_node(cur_type, name));
            add_var_to_symbol_table(arg_table, name, cur_type);
        }
        consume_op(")");
    }
    // alloc new node
    Node *node = new_function_node(
        return_type, name, args, compound_stmt(), var_offset, arg_table);
    return node;
}

/*
    translation-unit ::= (function-definition | declaration)+
*/
NodeList *translation_unit() {
    NodeList *result = calloc(1, sizeof(NodeList));
    while(cur_token->type != TK_EOF) {
        if(is_func_def()) append_node_list(result, function_definition());
        else append_node_list(result, declaration());
        // init var offset
        var_offset = 0;
    }
    return result;
}

Program *parse(Token *token_list) {
    // init token stream
    cur_token = token_list;
    Program *program = calloc(1, sizeof(Program));
    // establish symbol table and parse
    program->table = push_symbol_table();
    program->tree = translation_unit();
    return program;
}
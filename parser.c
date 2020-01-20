#include "compiler.h"

/* help function */
void print_tree(Node *cur_node) {
    switch(cur_node->type) {
        /* constant */
        case ND_INT:
            printf("<int>");
            printf("%d", cur_node->extend.val);
            printf("</int>\n");
            break;
        /* unary op */
        case ND_BIT_NOT:
            printf("<bit_not>\n");
            print_tree(cur_node->extend.expr);
            printf("</bit_not>\n");
            break;
        case ND_LOGIC_NOT:
            printf("<logic_not>\n");
            print_tree(cur_node->extend.expr);
            printf("</logic_not>\n");
            break;
        /* bi op */
        case ND_ADD:
            printf("<add>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</add>\n");
            break;
        case ND_SUB:
            printf("<sub>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</sub>\n");
            break;
        case ND_MUL:
            printf("<mul>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</mul>\n");
            break;
        case ND_DIV:
            printf("<div>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</div>\n");
            break;
        case ND_MOD:
            printf("<mod>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</mod>\n");
            break;
        case ND_NE:
            printf("<ne>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</ne>\n");
            break;
        case ND_EQ:
            printf("<eq>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</eq>\n");
            break;
        case ND_LT:
            printf("<lt>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</lt>\n");
            break;
        case ND_LE:
            printf("<le>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</le>\n");
            break;
        case ND_LSHIFT:
            printf("<lshift>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</lshift>\n");
            break;
        case ND_RSHIFT:
            printf("<rshift>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</rshift>\n");
            break;
        case ND_BIT_AND:
            printf("<bit_and>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</bit_and>\n");
            break;
        case ND_LOGIC_AND:
            printf("<logic_and>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</logic_and>\n");
            break;
        case ND_LOGIC_OR:
            printf("<logic_or>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</logic_or>\n");
            break;
        case ND_BIT_OR:
            printf("<bit_or>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</bit_or>\n");
            break;
        case ND_BIT_XOR:
            printf("<bit_xor>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</bit_xor>\n");
            break;
        /* ternary op */
        case ND_IF:
            printf("<if>\n");
            printf("<condition>\n");
            print_tree(cur_node->extend.ternode.condition);
            printf("</condition>\n");
            printf("<if_true>\n");
            print_tree(cur_node->extend.ternode.if_stmt);
            printf("</if_true>\n");
            printf("<if_false>\n");
            print_tree(cur_node->extend.ternode.else_stmt);
            printf("</if_false>\n");
            printf("</if>\n");
            break;
        /* special */
        case ND_BLOCK: {
            printf("<block>\n");
            NodeList *cur_node_list = cur_node->extend.stmts;
            while(cur_node_list) {
                printf("<stmt>\n");
                print_tree(cur_node_list->tree);
                printf("</stmt>\n");
                cur_node_list = cur_node_list->next;
            }
            printf("</block>\n");
            break;
        }
        case ND_RETURN:
            printf("<return>\n");
            print_tree(cur_node->extend.expr);
            printf("</return>\n");
            break;
        case ND_BREAK:
            printf("<break></break>\n");
            break;
        case ND_CONTINUE:
            printf("<continue></continue>\n");
            break;
        default:
            printf("print tree function has not defined %d", cur_node->type);
    }
}

void next_token() {
    cur_token = cur_token->next;
}

const int jump_keyword_len = 4;
const char *jump_keyword[4] = {
    "goto", "continue",
    "break", "return"
};
bool next_is_jump() {
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
    if( cur_token->type == TK_PUNC &&
        cur_token->len == strlen(str) &&
        strncmp(cur_token->str, str, cur_token->len)) {
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

/* check special op */
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

Node *add_node_block(NodeList *stmts){
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = ND_BLOCK;
    new_node->extend.stmts = stmts;
    return new_node;
}

/* parse function */
Node *stmt();
Node *jump_stmt();
NodeList *compound_stmt();
Node *declaration();
Node *direct_declarator();
Node *expr_stmt();
Node *expr();
Node *assign();
Node *condition();
Node *logic_or();
Node *logic_and();
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
<primary-expression> ::= <identifier>
                       | <constant> incomplete
                       | <string>
                       | ( <expression> ) ok 
*/
Node *primary() {
    Node *cur_node;
    // try constant
    if(cur_token->type == TK_CONSTANT) {
        //TODO: support different type
        cur_node = add_node_int(atoi(cur_token->str));
        next_token();
        return cur_node;
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

/*
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

/*
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

/* <direct-declarator> ::= <identifier>
                      | ( <declarator> )
                      | <direct-declarator> [ {<constant-expression>}? ]
                      | <direct-declarator> ( <parameter-type-list> )
                      | <direct-declarator> ( {<identifier>}* )
*/
Node *direct_declarator() {

}

/* <declarator> ::= {<pointer>}? <direct-declarator> */
Node *declaration() {

}

/*
<compound-statement> ::= { {<declaration>}* {<statement>}* }
*/
NodeList *compound_stmt() {
    // TODO: support declaration
    /* warning: "{" has been consumed at stmt stage */
    NodeList *node_list = NULL;
    NodeList *cur_node;
    while(!consume_op("}")) {
        if(!node_list) {
            node_list = calloc(1, sizeof(NodeList));
            cur_node = node_list;
            cur_node->tree = stmt();
        } else {
            NodeList *new_node = calloc(1, sizeof(NodeList));
            new_node->tree = stmt();
            cur_node->next = new_node;
            cur_node = new_node;
        }
    }
    return node_list;
}

/* <jump-statement> ::= goto <identifier> ;
                   | continue ; ok
                   | break ; ok
                   | return {<expression>}? ; ok
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
        return add_node_unary(ND_RETURN, expr());
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
        return add_node_block(compound_stmt());
    } else if(next_is_jump()) {
        return jump_stmt();
    }
    return expr_stmt();
}

Node *parse() {
    return stmt();
}
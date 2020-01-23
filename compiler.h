#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// tokenizer.c
typedef struct Token Token;
typedef enum TokenType TokenType;

enum TokenType{
    TK_PUNC,
    TK_IDENT,
    TK_KEYWORD,
    TK_CONSTANT,
    TK_STRING,
    TK_EOF,
};

struct Token {
    TokenType type;
    Token *next;
    char *str;
    int len;
};

// parser.c

typedef struct TerNode TerNode;
typedef struct BiNode BiNode;
typedef struct BlockNode BlockNode;
typedef struct LoopNode LoopNode;
typedef struct FunctionNode FunctionNode;
typedef struct Node Node;
typedef enum NodeType NodeType;
typedef union NodeExtend NodeExtend;
typedef struct NodeList NodeList;
typedef struct Var Var;
typedef struct SymbolTableList SymbolTableList;
typedef struct SymbolTable SymbolTable;

/*
   (SymbolTable.inner)
function -> scope 1 -> scope 3
              | (SymbolTableList.next)
              V
            scope 2 -> scope 5
*/
struct SymbolTableList {
    SymbolTable *table;
    SymbolTableList *next;
};

struct Var {
    Var *next;
    char *name;
    int offset;
};

struct SymbolTable {
    SymbolTableList *inner;
    SymbolTable *prev;
    Var *var;
    int cnt;
};

struct BiNode {
    Node *lhs, *rhs;
};

struct TerNode {
    Node *condition, *if_stmt, *else_stmt;
};

struct BlockNode {
    NodeList *stmts;
    SymbolTable *symbol_table;
};

struct LoopNode {
    Node *init;
    Node *condition;
    Node *stmt;
    Node *after_check;
};

struct FunctionNode {
    Node *stmt;
    SymbolTable *symbol_table;
    char *name;
};

union NodeExtend{
	int val; // ND_INT
    Node *expr; // unary operation (* &), return
    BiNode binode; // bi operation
    TerNode ternode; // ter operation(if else)
    BlockNode blocknode; // compoud stmt
    LoopNode loopnode; // for while
    FunctionNode functionnode; // for function declare
    char *name; // ident
};

enum NodeType {
    /* constant */
    ND_INT,
    /* ident */
    ND_IDENT,
    /* unary op */
    ND_BIT_NOT, // ~
    ND_LOGIC_NOT, // !
    /* bi op */
    ND_DECLARE, // declare variable
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // num * num
    ND_DIV, // num / num
    ND_MOD, // %
    ND_NE, // !=
    ND_EQ, // ==
    ND_LT, // <    also can use in >
    ND_LE, // <=   also can use in >=
    ND_LSHIFT, // <<
    ND_RSHIFT, // >>
    ND_BIT_AND, // &
    ND_BIT_OR, // |
    ND_BIT_XOR, // ^
    ND_LOGIC_AND, // &&
    ND_LOGIC_OR, // ||
    /* ter op*/
    ND_IF, // cond ? if_stmt : else_stmt | if(...) ... else ...
    /* special */
    ND_BLOCK, // {...}
    ND_LOOP, // for, while
    ND_DO_LOOP, // do while
    ND_FUNCTION, // definition of function
    ND_CALL, // function call
    /* jmp */
    ND_BREAK, // break
    ND_CONTINUE, // continue
    ND_RETURN, // return
    /* assign op node */
    ND_ASSIGN,
};

struct Node {
    NodeType type;
    NodeExtend extend;
};

struct NodeList {
    Node *tree;
    NodeList *next;
};

// global
Token *token_list;
Token *cur_token;

SymbolTable *cur_symbol_table;
SymbolTable *symbol_table_head;

NodeList *function_list;

/* main function */
void tokenize(FILE*);
void parse();
void codegen(NodeList*);
Var *find_var(SymbolTable*,char*);
int count_symbol_table(SymbolTable*);
char *get_ident_name(Node*);

/* debug function */
void print_cur_token(Token*);
void print_token();
void print_tree(Node*);
void print_symbol_table(SymbolTable*);
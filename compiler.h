#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// tokenizer.c
typedef struct Token Token;
typedef enum TokenType TokenType;

enum TokenType{
    // kw token
    TK_KW_BREAK,
    TK_KW_CHAR,
    TK_KW_CONTINUE,
    TK_KW_ELSE,
    TK_KW_FOR,
    TK_KW_IF,
    TK_KW_INT,
    TK_KW_RETURN,
    TK_KW_WHILE,
    // TODO: following token haven't been supported
    TK_KW_SIGNED,
    TK_KW_UNSIGNED,
    TK_KW_DOUBLE,
    TK_KW_FLOAT,
    TK_KW_SHORT,
    TK_KW_VOID,

    TK_KW_GOTO,
    TK_KW_SWITCH,
    TK_KW_DO,
    TK_KW_CASE,

    TK_KW_UNDEFINED, // to specified the token that is not supported
    // another
    TK_PUNC,
    TK_IDENT,
    TK_INT,
    TK_FLOAT,
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
typedef struct Program Program;
// Node type
typedef struct UnNode UnNode;
typedef struct BiNode BiNode;
typedef struct TerNode TerNode;
typedef struct LoopNode LoopNode;
typedef struct DeclNode DeclNode;
typedef struct CallNode CallNode;

typedef struct BlockNode BlockNode;
typedef struct FunctionNode FunctionNode;
typedef struct Node Node;
typedef enum NodeType NodeType;
typedef union NodeExtend NodeExtend;
typedef struct NodeList NodeList;
typedef struct Var Var;
typedef struct SymbolTable SymbolTable;

struct Program {
    SymbolTable *table;
    NodeList *tree;
};

typedef struct Type Type;
/*
    cur_symbol_table
          |
          V
        scope 1 -> scope 2 -> function -> global
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

struct UnNode {
    Node *next;
};

struct BiNode {
    Node *lhs, *rhs;
};

struct TerNode {
    Node *condition, *if_stmt, *else_stmt;
};

struct LoopNode {
    Node *init, *cond, *after, *stmt;
};

struct DeclNode {
    Type *type;
    char *name;
};

struct CallNode {
    NodeList *arg_list;
    Node *callee;
};


struct BlockNode {
    NodeList *stmts;
    SymbolTable *symbol_table;
};



struct FunctionNode {
    Node *stmt;
    SymbolTable *symbol_table;
    int memory;
    char *name;
    Type *return_type;
};

// see node type
union NodeExtend{
    UnNode unnode;
    BiNode binode;
    TerNode ternode;
    LoopNode loopnode;
    DeclNode declnode;
    CallNode callnode;


    FunctionNode functionnode;
	int val; // ND_INT
    Node *expr; // unary operation (* &), return
    BlockNode blocknode; // compoud stmt
    char *name; // ident
};

enum NodeType {
    /* unary node */
    ND_POST_INC, // id ++
    ND_POST_DEC, // id --
    ND_PRE_INC, // ++ id
    ND_PRE_DEC, // -- id
    ND_ADDR, // &
    ND_DEREF, // *
    ND_BIT_NOT, // ~
    ND_LOGIC_NOT, // !
    ND_RETURN, // return
    ND_BREAK, // break
    ND_CONTINUE, // continue
    /* bi node */
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
    ND_ASSIGN,
    /* ter node */
    ND_IF, // cond ? if_stmt : else_stmt | if(...) ... else ...
    /* loop node */
    ND_LOOP, // for, while
    ND_DO_LOOP, // do while
    /* decl node */
    ND_DECLARE, // declare variable
    /* call node */
    ND_CALL, // function call




    /* constant */
    ND_INT,
    /* ident */
    ND_IDENT,
    /* special */
    ND_BLOCK, // {...}
    ND_FUNCTION, // definition of function
};

struct Node {
    NodeType type;
    NodeExtend extend;
};

struct NodeList {
    Node *tree;
    NodeList *next;
};

/* type.c */
struct Type {
    int size;
    int aligned;
};

Type INT_TYPE;
Type CHAR_TYPE;

/* main function */
Token *tokenize(FILE*);
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
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

typedef struct BiNode BiNode;
typedef struct Node Node;
typedef enum NodeType NodeType;
typedef union NodeExtend NodeExtend;
typedef struct NodeList NodeList;

struct BiNode {
    Node *lhs, *rhs;
};

union NodeExtend{
	int val; // ND_INT
    Node *expr; // * &
    BiNode binode; // bi operation
};

enum NodeType {
    // constant
    ND_INT,
    /* unary op */
    ND_BIT_NOT, // ~
    ND_LOGIC_NOT, // !
    /* bi op */
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
    ND_LOGIC_AND, // &&
    ND_LOGIC_OR, // ||
    ND_INCLUSIVE_OR, // |
    ND_EXCLUSIVE_OR, // ^
    /* assign op node */
    ND_ASSIGN,
    ND_MUL_ASSIGN,
    ND_DIV_ASSIGN,
    ND_MOD_ASSIGN,
    ND_ADD_ASSIGN,
    ND_SUB_ASSIGN,
    ND_LSHIFT_ASSIGN,
    ND_RSHIFT_ASSIGN,
    ND_AND_ASSIGN,
    ND_XOR_ASSIGN,
    ND_OR_ASSIGN,
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


void tokenize(FILE*);
NodeList *parse();

/* code gen */
void codegen(NodeList*);
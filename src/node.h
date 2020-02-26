#ifndef NODE_H
#define NODE_H

#include "type.h"
#include "symbolTable.h"

/* top level */
typedef struct Program Program;
/* basic node */
typedef struct Node Node;
typedef enum NodeType NodeType;
typedef union NodeExtend NodeExtend;
/* extend node struct */
typedef struct UnNode UnNode;
typedef struct BiNode BiNode;
typedef struct TerNode TerNode;
typedef struct LoopNode LoopNode;
typedef struct DeclNode DeclNode;
typedef struct CallNode CallNode;
typedef struct BlockNode BlockNode;
typedef struct FunctionNode FunctionNode;
typedef struct NodeList NodeList;
/* node type enum */
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
    ND_GOTO, // goto
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
    /* function node */
    ND_FUNC, // function definition

    /* constant */
    ND_INT,
    /* ident */
    ND_IDENT,
    /* special */
    ND_BLOCK, // {...}
};


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



struct UnNode {
    Node *next;
};

struct BiNode {
    Node *lhs, *rhs;
};

struct TerNode {
    Node *cond, *if_stmt, *else_stmt;
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

struct FunctionNode {
    Type *return_type;
    NodeList *arg_list;
    Node *stmt;
    SymbolTable *arg_table;
    int memory;
    char *name;
};

struct BlockNode {
    NodeList *stmts;
    SymbolTable *symbol_table;
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
struct Node {
    NodeType type;
    NodeExtend extend;
};

struct NodeList {
    Node *tree;
    NodeList *next;
};

/* node constructor */
Node *new_int_node(int);
Node *new_unary_node(NodeType, Node*);
Node *new_binary_node(NodeType, Node*, Node*);
Node *new_ternary_node(Node*, Node*, Node*);
Node *new_loop_node(NodeType, Node*, Node*, Node*, Node*);
Node *new_decl_node(Type*, char*);
Node *new_call_node(Node *callee, NodeList *arg_list);
Node *new_function_node(Type*, char*, NodeList*, Node*, int , SymbolTable*);
Node *add_node_block(NodeList*, SymbolTable*);
Node *add_node_ident(char*);




#endif
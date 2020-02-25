#include <stdlib.h>
#include "node.h"

Node *new_int_node(int val) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = ND_INT;
    new_node->extend.val = val;
    return new_node;
}

Node *new_unary_node(NodeType type, Node *expr) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = type;
    new_node->extend.unnode.next = expr;
    return new_node;
}

Node *new_binary_node(NodeType type, Node *lhs, Node *rhs){
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = type;
    new_node->extend.binode.lhs = lhs;
    new_node->extend.binode.rhs = rhs;
    return new_node;
}

Node *new_ternary_node(Node *cond, Node *if_stmt, Node *else_stmt) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = ND_IF;
    new_node->extend.ternode.cond = cond;
    new_node->extend.ternode.if_stmt = if_stmt;
    new_node->extend.ternode.else_stmt = else_stmt;
    return new_node;
}

Node *new_loop_node(NodeType type, Node *init, Node *cur_cond, Node *cur_stmt, Node *after) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = type;
    new_node->extend.loopnode.init = init;
    new_node->extend.loopnode.cond = cur_cond;
    new_node->extend.loopnode.stmt = cur_stmt;
    new_node->extend.loopnode.after = after;
    return new_node;
}

Node *new_decl_node(Type *type, char *name) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = ND_DECLARE;
    new_node->extend.declnode.name = name;
    new_node->extend.declnode.type = type;
    return new_node;
}

Node *new_call_node(Node *callee, NodeList *arg_list) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = ND_CALL;
    new_node->extend.callnode.arg_list = arg_list;
    new_node->extend.callnode.callee = callee;
    return new_node;
}

Node *new_function_node(
    Type *return_type, char *name, NodeList *args, Node *stmt, int memory, SymbolTable* arg_table
) {
    Node *new_node = calloc(1, sizeof(Node));
    new_node->type = ND_FUNC;
    new_node->extend.functionnode.return_type = return_type;
    new_node->extend.functionnode.name = name;
    new_node->extend.functionnode.arg_list = args;
    new_node->extend.functionnode.stmt = stmt;

    new_node->extend.functionnode.memory = memory;
    new_node->extend.functionnode.arg_table = arg_table;
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
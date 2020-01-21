#include "compiler.h"

SymbolTable *gen_symbol_table;

/* pop function */
void normal_pop() {
    printf("  pop rdi\n");
    printf("  pop rax\n");
}

void shift_pop() {
    printf("  pop rcx\n");
    printf("  pop rax\n");
}

void logic_pop() {
    // b to logic
    printf("  pop rdx\n");
    printf("  cmp rdx, 0\n");
    printf("  setne dl\n");
    // a to logic
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  setne al\n");
}

/* gen function */
void gen(Node *cur_node) {
    // declare
    // TODO: if has init stmt then do something
    if(cur_node->type == ND_DECLARE) {
        return;
    }
    // constant should push
    if(cur_node->type == ND_INT) {
        printf("  push %d\n", cur_node->extend.val);
        return;
    }
    // ident
    if(cur_node->type == ND_IDENT) {
        Var *cur_var = find_var(gen_symbol_table, cur_node->extend.name);
        if(cur_var) {
            printf("  offset %d\n", cur_var->offset);
        } else {
            printf("  not exist\n");
            exit(1);
        }
        return;
    }
    // block node
    if(cur_node->type == ND_BLOCK) {
        gen_symbol_table = cur_node->extend.blocknode.symbol_table;
        NodeList *cur_node_list = cur_node->extend.blocknode.stmts;
        while(cur_node_list) {
            gen(cur_node_list->tree);
            cur_node_list = cur_node_list->next;
        }
        gen_symbol_table = cur_node->extend.blocknode.symbol_table->prev;
        return;
    }
    // bi operation
    gen(cur_node->extend.binode.lhs);
    gen(cur_node->extend.binode.rhs);
    switch(cur_node->type) {
        // shift op
        case ND_LSHIFT:
            shift_pop();
            printf("  shl rax, cl\n");
            break;
        case ND_RSHIFT:
            shift_pop();
            printf("  shr rax, cl\n");
            break;
        // relational op
        case ND_LT:
            normal_pop();
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            break;
        case ND_LE:
            normal_pop();
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            break;
        case ND_EQ:
            normal_pop();
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            break;
        case ND_NE:
            normal_pop();
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            break;
        // arithmetic op
        case ND_ADD:
            normal_pop();
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            normal_pop();
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            normal_pop();
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            normal_pop();
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case ND_MOD:
            normal_pop();
            printf("  cqo\n");
            printf("  idiv rdi\n");
            printf("  mov rax, rdx\n");
            break;
        // bit op
        case ND_BIT_AND:
            normal_pop();
            printf("  and rax, rdi\n");
            break;
        case ND_BIT_OR:
            normal_pop();
            printf("  or rax, rdi\n");
            break;
        case ND_BIT_XOR:
            normal_pop();
            printf("  xor rax, rdi\n");
            break;
        // logic op
        case ND_LOGIC_AND:
            logic_pop();
            printf("  and rax, rdx\n");
            break;
        case ND_LOGIC_OR:
            logic_pop();
            printf("  or rax, rdx\n");
            break;
    }
    printf("  push rax\n");
}

void codegen(Node *node_list) {
    printf(".intel_syntax noprefix\n");
    printf(".globl _main\n");
    // TODO: support function
    printf("_main:\n");
    gen(node_list);
    printf("  pop rax\n");
    printf("  ret\n");
}
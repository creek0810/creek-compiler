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
void gen_addr(int delta) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", delta);
}

void gen(Node *cur_node) {
    // terminal node
    switch(cur_node->type) {
        case ND_RETURN: {
            gen(cur_node->extend.expr);
            printf("  pop rax\n");
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;
        }
        // TODO: if has init stmt then do something
        case ND_DECLARE:
            return;
        case ND_IDENT: {
            Var *cur_var = find_var(gen_symbol_table, cur_node->extend.name);
            if(cur_var) {
                gen_addr((cur_var->offset + 1) * 8);
                printf("  push [rax]\n");
            } else {
                printf("  not exist\n");
                exit(1);
            }
            return;
        }
        case ND_ASSIGN: {
            gen(cur_node->extend.binode.rhs);
            Var *cur_var = find_var(gen_symbol_table, cur_node->extend.binode.lhs->extend.name);
            // calc lvar offset and store it into rax
            gen_addr((cur_var->offset + 1) * 8);
            printf("  pop rdi\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n"); // left association
            return;
        }
        case ND_BLOCK: {
            gen_symbol_table = cur_node->extend.blocknode.symbol_table;
            NodeList *cur_node_list = cur_node->extend.blocknode.stmts;
            while(cur_node_list) {
                gen(cur_node_list->tree);
                cur_node_list = cur_node_list->next;
            }
            gen_symbol_table = cur_node->extend.blocknode.symbol_table->prev;
            return;
        }
        case ND_INT: {
            printf("  push %d\n", cur_node->extend.val);
            return;
        }
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
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");

    // TODO: support function local var
    printf("  sub rsp, %d\n", count_symbol_table(symbol_table_head) * 8);

    gen(node_list);
    // return val
    printf("  pop rax\n");
    // restore
    // TODO: if no return stmt then auto return
    /*
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    */
    printf("  ret\n");
}
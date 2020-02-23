#include "compiler.h"

SymbolTable *gen_symbol_table;
int IF_CNT = 0;
int LOOP_CNT = 0;


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
void gen_addr(Var *cur_var) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", cur_var->offset);
    int aligned = cur_var->type->aligned;
    if(aligned == 1) {
        printf("  movsx rax, byte ptr [rax]\n");
    }else if(aligned == 4) {
        printf("  movsx rax, word ptr [rax]\n");
    } else if(aligned == 8) {
        printf("  mov rax, [rax]\n");
    }
    printf("  push rax\n");
}

void store(Var *cur_var) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", cur_var->offset);
    printf("  pop rdx\n");
    int aligned = cur_var->type->aligned;
    if(aligned == 1) {
        printf("  mov [rax], dl\n");
    } else if(aligned == 4) {
        printf("  mov [rax], edx\n");
    } else if(aligned == 8) {
        printf("  mov [rax], rdx\n");
    }
    printf("  push rdx\n"); // left association
}

void gen(Node *cur_node) {
    // printf("type: %d\n", cur_node->type);
    if(cur_node == NULL) {
        return;
    }
    // terminal node
    switch(cur_node->type) {
        case ND_CONTINUE: {
            printf("  jmp LOOP_START_%d\n", LOOP_CNT - 1);
            return;
        }
        case ND_BREAK: {
            printf("  jmp LOOP_END_%d\n", LOOP_CNT - 1);
            return;
        }
        case ND_CALL: {
            printf("  call %s\n", cur_node->extend.binode.lhs->extend.name);
            printf("  push rax\n");
            return;
        }
        case ND_FUNCTION: {
            // TODO: support function
            if( strlen(cur_node->extend.functionnode.name) == 4 &&
                strncmp(cur_node->extend.functionnode.name, "main", 4) == 0
                ) {
                printf("_main:\n");
            } else {
                printf("%s:\n", cur_node->extend.functionnode.name);
            }
            // establish base env
            printf("  push rbp\n");
            printf("  mov rbp, rsp\n");
            // alloc memory
            printf("  sub rsp, %d\n", cur_node->extend.functionnode.memory);
            // gen code
            gen(cur_node->extend.functionnode.stmt);
            return;
        }
        case ND_DO_LOOP: {
            int cur_loop_idx = LOOP_CNT++;
            gen(cur_node->extend.loopnode.stmt);
            printf("LOOP_START_%d:\n", cur_loop_idx);
            gen(cur_node->extend.loopnode.condition);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je LOOP_END_%d\n", cur_loop_idx);
            gen(cur_node->extend.loopnode.stmt);
            printf("  jmp LOOP_START_%d\n", cur_loop_idx);
            printf("LOOP_END_%d:\n", cur_loop_idx);
            return;
        }
        case ND_LOOP: {
            int cur_loop_idx = LOOP_CNT++;
            gen(cur_node->extend.loopnode.init);
            printf("LOOP_START_%d:\n", cur_loop_idx);
            gen(cur_node->extend.loopnode.condition);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je LOOP_END_%d\n", cur_loop_idx);
            gen(cur_node->extend.loopnode.stmt);
            gen(cur_node->extend.loopnode.after_check);
            printf("  jmp LOOP_START_%d\n", cur_loop_idx);
            printf("LOOP_END_%d:\n", cur_loop_idx);
            return;
        }
        case ND_RETURN: {
            gen(cur_node->extend.expr);
            printf("  pop rax\n");
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;
        }
        case ND_DECLARE: {
            if(cur_node->extend.binode.rhs->type == ND_ASSIGN) {
                gen(cur_node->extend.binode.rhs);
            }
            return;
        }
        case ND_IDENT: {
            Var *cur_var = find_var(gen_symbol_table, cur_node->extend.name);
            if(cur_var) {
                gen_addr(cur_var);
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
            store(cur_var);
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
        case ND_IF: {
            int if_id = IF_CNT++;
            gen(cur_node->extend.ternode.condition);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je IF_FALSE_%d\n", if_id);
            // if true
            gen(cur_node->extend.ternode.if_stmt);
            printf("  jmp IF_END_%d\n", if_id);
            // if false
            printf("IF_FALSE_%d:\n", if_id);
            if(cur_node->extend.ternode.else_stmt) {
                gen(cur_node->extend.ternode.else_stmt);
            }
            printf("IF_END_%d:\n", if_id);
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

void codegen(NodeList *node_list, SymbolTable *global_table) {
    // store global symbol table
    gen_symbol_table = global_table;

    printf(".intel_syntax noprefix\n");
    printf(".globl _main\n");
    while(node_list) {
        gen(node_list->tree);
        node_list = node_list->next;
        printf("\n");
    }

    // restore
    // TODO: if no return stmt then auto return
    /*
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    */
}
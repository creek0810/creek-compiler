#include "compiler.h"
#include "symbolTable.h"

SymbolTable *gen_symbol_table;
NodeList *arg_symbol_table;
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
void gen_local_addr(Var *cur_var) {
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

void gen_parameter_addr(int idx) {
    printf("  mov rax, rbp\n");
    printf("  add rax, %d\n", (1 + idx) * 8);
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
}

// TODO: finish it!
void gen_global_addr(Var *cur_var) {
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


void gen(Node *cur_node);
void gen_bi_node(Node *cur_node);
void gen_ter_node(Node *cur_node);
void gen_loop_node(Node *cur_node);

void gen(Node *cur_node) {
    if(cur_node == NULL) return;
    // terminal node
    switch(cur_node->type) {
        case ND_DO_LOOP:
        case ND_LOOP:
            gen_loop_node(cur_node);
            return;
        case ND_IF:
            gen_ter_node(cur_node);
            return;


        
        case ND_CONTINUE: {
            printf("  jmp LOOP_START_%d\n", LOOP_CNT - 1);
            return;
        }
        case ND_BREAK: {
            printf("  jmp LOOP_END_%d\n", LOOP_CNT - 1);
            return;
        }
        case ND_CALL: {
            // push arg
            NodeList *cur_list = cur_node->extend.callnode.arg_list;
            while(cur_list) {
                gen(cur_list->tree);
                cur_list = cur_list->next;
            }

            // start call
            printf("  call %s\n", cur_node->extend.callnode.callee->extend.name);
            printf("  push rax\n");
            return;
        }
        case ND_FUNC: {
            // TODO: support function
            arg_symbol_table = cur_node->extend.functionnode.arg_list;
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
                gen_local_addr(cur_var);
            } else {
                // find in arg table
                NodeList *arg_list = arg_symbol_table;
                int idx = 1;
                bool found = false;
                char *tar_var_name = cur_node->extend.name;
                int tar_len = strlen(tar_var_name);
                while(arg_list) {
                    char *cur_var_name = arg_list->tree->extend.declnode.name;
                    if(tar_len == strlen(cur_var_name) &&
                        strncmp(tar_var_name, cur_var_name, tar_len) == 0) {
                        found = true;
                        break;
                    }
                    arg_list = arg_list->next;
                    idx ++;
                }

                if(found) {
                    gen_parameter_addr(idx);
                } else {
                    printf("  not exist\n");
                    exit(1);
                }
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
        
        default:
            gen_bi_node(cur_node);
            return;
    }
}

void gen_loop_node(Node *cur_node) {
    int cur_loop_idx = LOOP_CNT++;
    /* do loop need to execute stmt once at first */
    switch(cur_node->type){
        case ND_DO_LOOP:
            gen(cur_node->extend.loopnode.stmt);
        case ND_LOOP:
            gen(cur_node->extend.loopnode.init);
            printf("LOOP_START_%d:\n", cur_loop_idx);
            gen(cur_node->extend.loopnode.cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je LOOP_END_%d\n", cur_loop_idx);
            gen(cur_node->extend.loopnode.stmt);
            gen(cur_node->extend.loopnode.after);
            printf("  jmp LOOP_START_%d\n", cur_loop_idx);
            printf("LOOP_END_%d:\n", cur_loop_idx);
            return;
        default:
            printf("gen_loop_node error: not a loop node!\n");
    }
}
void gen_ter_node(Node *cur_node) {
    int if_id = IF_CNT++;
    gen(cur_node->extend.ternode.cond);
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






void gen_bi_node(Node *cur_node) {
    if(cur_node == NULL) return;
    // start gen
    gen(cur_node->extend.binode.lhs);
    gen(cur_node->extend.binode.rhs);
    switch(cur_node->type) {
        /*
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
        */





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

void codegen(Program *program) {
    // load global symbol table
    gen_symbol_table = program->table;
    printf(".intel_syntax noprefix\n");
    printf(".globl _main\n");
    NodeList *node_list = program->tree;
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
#include "compiler.h"

bool SCANNER_DEBUG = false;
bool PARSER_DEBUG = false;

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("參數錯誤\n");
        return 1;
    }
    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL) {
        printf("檔案不存在，或無法開啟");
        return 1;
    }
    // scanner
    Token *token_list = scan(fp);
    if(SCANNER_DEBUG) print_token(token_list);
    // parser
    Program *program = parse(token_list);
    if(PARSER_DEBUG) {
        NodeList *cur_function = program->tree;
        while(cur_function) {
            print_tree(cur_function->tree);
            cur_function = cur_function->next;
        }
    }
    /* debug parser */
    // printf("-------\n", symbol_table_head);
    // print_symbol_table(symbol_table_head);

    /* start gen */
    codegen(program);
}
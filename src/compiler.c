#include "compiler.h"
#include <string.h>

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("參數錯誤\n");
        return 1;
    }
    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL) {
        printf("檔案不存在，或無法開啟");
        return 1;
    }
    if(argc == 3 && strlen(argv[2]) == 4 && strncmp(argv[2], "scan", 4) == 0) {
        Token *token_list = scan(fp);
        print_token(token_list);
    } else {
        Token *token_list = scan(fp);
        Program *program = parse(token_list);
        codegen(program);
        /*
        if(PARSER_DEBUG) {
            NodeList *cur_function = program->tree;
            while(cur_function) {
                print_tree(cur_function->tree);
                cur_function = cur_function->next;
            }
        }
        */
    }
}
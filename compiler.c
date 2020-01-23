#include "compiler.h"

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
    tokenize(fp);
    /* debug tokenizer */
    // print_token();

    cur_token = token_list;
    parse();



    NodeList *cur_function = function_list;
    /* debug parser */
    /*
    while(cur_function) {
        print_tree(cur_function->tree);
        cur_function = cur_function->next;
    }
    */
    // printf("-------\n", symbol_table_head);
    // print_symbol_table(symbol_table_head);

    /* start gen */
    /*cur_function = function_list; */

    codegen(function_list);

}
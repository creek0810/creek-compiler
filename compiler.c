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
    NodeList *node_list = parse();

    /* debug parser */
    while(node_list) {
        print_tree(node_list->tree);
        node_list = node_list->next;
    }

}
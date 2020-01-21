#include "compiler.h"

void print_cur_token(Token *it) {
     switch(it->type) {
            case TK_PUNC:
                printf("punc: ");
                break;
            case TK_CONSTANT:
                printf("constant: ");
                break;
            case TK_STRING:
                printf("string literal: ");
                break;
            case TK_KEYWORD:
                printf("keyword: ");
                break;
            case TK_IDENT:
                printf("ident: ");
                break;
            case TK_EOF:
                printf("eof\n");
                return;
        }
        printf("%s %d\n", it->str, it->len);
}

void print_token() {
    Token *it = token_list;
    while(it) {
       print_cur_token(it);
        it = it->next;
    }
}

void print_var_list(Var *cur_var) {
    while(cur_var) {
        printf("<var>%s %d</var>\n", cur_var->name, cur_var->offset);
        cur_var = cur_var->next;
    }
}

void print_symbol_table(SymbolTable *cur_table) {
    /*
    (SymbolTable.inner)
    function -> scope 1 -> scope 3
                | (SymbolTableList.next)
                V
                scope 2 -> scope 5
    */
    if(!cur_table) {
        return;
    }
    printf("<block>\n");
    print_var_list(cur_table->var);
    SymbolTableList *cur_it_table = cur_table->inner;
    while(cur_it_table) {
        print_symbol_table(cur_it_table->table);
        cur_it_table = cur_it_table->next;
    }
    printf("</block>\n");
}

void print_tree(Node *cur_node) {
    switch(cur_node->type) {
        /* constant */
        case ND_INT:
            printf("<int>");
            printf("%d", cur_node->extend.val);
            printf("</int>\n");
            break;
        case ND_IDENT:
            printf("<ident>");
            printf("%s", cur_node->extend.name);
            printf("</ident>\n");
            break;
        /* unary op */
        case ND_BIT_NOT:
            printf("<bit_not>\n");
            print_tree(cur_node->extend.expr);
            printf("</bit_not>\n");
            break;
        case ND_LOGIC_NOT:
            printf("<logic_not>\n");
            print_tree(cur_node->extend.expr);
            printf("</logic_not>\n");
            break;
        /* bi op */
        case ND_ASSIGN:
            printf("<assign>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</assign>\n");
            break;
        case ND_ADD:
            printf("<add>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</add>\n");
            break;
        case ND_SUB:
            printf("<sub>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</sub>\n");
            break;
        case ND_MUL:
            printf("<mul>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</mul>\n");
            break;
        case ND_DIV:
            printf("<div>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</div>\n");
            break;
        case ND_MOD:
            printf("<mod>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</mod>\n");
            break;
        case ND_NE:
            printf("<ne>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</ne>\n");
            break;
        case ND_EQ:
            printf("<eq>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</eq>\n");
            break;
        case ND_LT:
            printf("<lt>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</lt>\n");
            break;
        case ND_LE:
            printf("<le>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</le>\n");
            break;
        case ND_LSHIFT:
            printf("<lshift>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</lshift>\n");
            break;
        case ND_RSHIFT:
            printf("<rshift>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</rshift>\n");
            break;
        case ND_BIT_AND:
            printf("<bit_and>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</bit_and>\n");
            break;
        case ND_LOGIC_AND:
            printf("<logic_and>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</logic_and>\n");
            break;
        case ND_LOGIC_OR:
            printf("<logic_or>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</logic_or>\n");
            break;
        case ND_BIT_OR:
            printf("<bit_or>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</bit_or>\n");
            break;
        case ND_BIT_XOR:
            printf("<bit_xor>\n");
            print_tree(cur_node->extend.binode.lhs);
            print_tree(cur_node->extend.binode.rhs);
            printf("</bit_xor>\n");
            break;
        /* ternary op */
        case ND_IF:
            printf("<if>\n");
            printf("<condition>\n");
            print_tree(cur_node->extend.ternode.condition);
            printf("</condition>\n");
            printf("<if_true>\n");
            print_tree(cur_node->extend.ternode.if_stmt);
            printf("</if_true>\n");
            printf("<if_false>\n");
            print_tree(cur_node->extend.ternode.else_stmt);
            printf("</if_false>\n");
            printf("</if>\n");
            break;
        /* special */
        case ND_BLOCK: {
            printf("<block>\n");
            NodeList *cur_node_list = cur_node->extend.blocknode.stmts;
            while(cur_node_list) {
                printf("<stmt>\n");
                print_tree(cur_node_list->tree);
                printf("</stmt>\n");
                cur_node_list = cur_node_list->next;
            }
            printf("</block>\n");
            break;
        }
        case ND_RETURN:
            printf("<return>\n");
            print_tree(cur_node->extend.expr);
            printf("</return>\n");
            break;
        case ND_BREAK:
            printf("<break></break>\n");
            break;
        case ND_CONTINUE:
            printf("<continue></continue>\n");
            break;
        default:
            printf("print tree function has not defined %d", cur_node->type);
    }
}

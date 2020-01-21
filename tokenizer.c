#include "compiler.h"

/* warning: op need to be sorted by letters */
/* check more letters op first */
int punc_cnt= 54;
const char *punc_table[54] = {
    // more than 3 letters
    "%:%:", "<<=", ">>=", "...",
    // 2 letters op
    "->", "++", "--", "<:",
    "<<", ">>", "<=", ">=",
    "==", "!=", "&&", "||",
    "*=", "/=", "%=", ":>",
    "+=", "-=", "<%", "%>",
    "&=", "^=", "|=", "##",
    "%:",
    // 1 letter op
    "[", "]", "(", ")",
    "{", "}", ".", "&",
    "*", "+", "-", "~",
    "/", "%", "<", ">",
    "^", "|", "?", ":",
    ";", "=", ",", "#",
    "!",
};

int keyword_cnt = 44;
const char *keyword[44] = {
    "alignof",
    "auto",

    "break",

    "case",
    "char",
    "const",
    "continue",

    "default",
    "do",
    "double",

    "else",
    "enum",
    "extern",

    "float",
    "for",

    "goto",

    "if",
    "inline",
    "int",

    "long",

    "register",
    "restrict",
    "return",

    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
    "_Alignas",
    "_Atomic",
    "_Bool",
    "_Complex",
    "_Generic",
    "_Imaginary",
    "_Noreturn",
    "_Static_assert",
    "_Thread_loca",
};

/* help function */

int startswith(char *str, char *tar) {
    int str_len = strlen(str);
    int tar_len = strlen(tar);
    if(str_len >= tar_len) {
        return memcmp(str, tar, sizeof(char) * tar_len) == 0;
    }
    return 0;
}

void add_token(char *str, int len, TokenType type) {
    Token *new_token = calloc(1, sizeof(Token));
    new_token->type = type;
    new_token->next = NULL;
    // deal with str
    new_token->str = calloc(1, sizeof(char) * (len + 1));
    strncpy(new_token->str, str, len);

    new_token->len = len;
    if(cur_token == NULL) {
        cur_token = new_token;
        token_list = new_token;
    } else {
        cur_token->next = new_token;
        cur_token = new_token;
    }
}

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

/* token type function */

int punc(char *str, int base_loc, int str_len) {
    int rest_len = str_len - base_loc;
    for(int i=0; i<punc_cnt; i++) {
        int punc_len = strlen(punc_table[i]);
        if(rest_len >= punc_len &&
            strncmp(str + base_loc, punc_table[i], punc_len) == 0) {
            add_token(str + base_loc, punc_len, TK_PUNC);
            return base_loc + punc_len;
        }
    }
}

int ident(char *str, int base_loc, int str_len) {
    // we have checked first char is alpha or '_'
    int loc = base_loc + 1;


    // identifer and keyword can only be the following
    // alpha digit '_'
    while(loc < str_len &&
          (isalpha(str[loc]) || isdigit(str[loc]) || str[loc] == '_')) {
        loc++;
    }
    // check whether is keyword or identifer
    for(int i=0; i<keyword_cnt; i++) {
        int keyword_len = strlen(keyword[i]);
        // is keyword
        if(keyword_len == (loc - base_loc) &&
           strncmp(str + base_loc, keyword[i], keyword_len) == 0) {
            add_token(str + base_loc, keyword_len, TK_KEYWORD);
            return loc;
        }
    }
    // is identifer
    add_token(str + base_loc, loc - base_loc, TK_IDENT);
    return loc;
}

int constant(char *str, int base_loc, int str_len) {
    int loc = base_loc + 1;
    while(loc < str_len &&
          (isdigit(str[loc]) || str[loc] == '.')) {
        loc++;
    }
    add_token(str + base_loc, loc - base_loc, TK_CONSTANT);
    return loc;
}

int string_literal(char *str, int base_loc, int str_len) {
    int loc = base_loc + 1;
    while(loc < str_len) {
        loc++;
        // check if not a escape "
        if(str[loc] == '\"' && str[loc - 1] != '\\'){
            loc ++;
            break;
        }
    }
    add_token(str + base_loc, loc - base_loc, TK_STRING);
    return loc;
}

void tokenize(FILE *fp) {
    // the max len of a line should not exceed 5000!
    char str[5000] = {0};
    while(fgets(str, 5000, fp) != NULL) {
        int cur_loc = 0;
        int str_len = strlen(str);
        while(cur_loc < str_len) {
            // indent
            if(isspace(str[cur_loc])) {
                cur_loc ++;
                continue;
            }

            // inline comment
            if(startswith(str + cur_loc, "//")) {
                // printf("inline comment: %s\n", str + cur_loc);
                cur_loc = str_len;
                continue;
            }

            // identifier or keyword
            if(isalpha(str[cur_loc]) || str[cur_loc] == '_') {
                cur_loc = ident(str, cur_loc, str_len);
                continue;
            }

            // constant
            if(isdigit(str[cur_loc])) {
                cur_loc = constant(str, cur_loc, str_len);
                continue;
            }

            // string literal
            if(str[cur_loc] == '\"') {
                cur_loc = string_literal(str, cur_loc, str_len);
                continue;
            }

            // punctuator
            cur_loc = punc(str, cur_loc, str_len);
        }
    }
    add_token("", 0, TK_EOF);
}

